#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/TWSMUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"

#include "Game/GameCommon.hpp"


static float s_mouseSensitivityMultiplier = 1.f;


//-----------------------------------------------------------------------------------------------
Game::Game()
{
} 


//-----------------------------------------------------------------------------------------------
Game::~Game()
{
}


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	Transform::s_axisOrientation.InvertZAxis();
	g_eventSystem->RegisterEvent( "set_mouse_sensitivity", "Usage: set_mouse_sensitivity multiplier=NUMBER. Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );
	g_eventSystem->RegisterMethodEvent( "light_set_ambient_color", "Usage: light_set_ambient_color color=r,g,b", eUsageLocation::DEV_CONSOLE, this, &Game::SetAmbientLightColor );
	g_eventSystem->RegisterMethodEvent( "light_set_color", "Usage: light_set_color color=r,g,b", eUsageLocation::DEV_CONSOLE, this, &Game::SetPointLightColor );
	g_eventSystem->RegisterMethodEvent( "unsubscribe_game", "Usage: Unsubscribe game methods from event system for demo", eUsageLocation::DEV_CONSOLE, this, &Game::UnSubscribeGameMethods );

	g_eventSystem->RegisterMethodEvent( "load_obj_file", "Usage: load_obj_file path=<filepath relative to Run/Data/>", eUsageLocation::DEV_CONSOLE, this, &Game::LoadObjFile );
	g_eventSystem->RegisterMethodEvent( "load_twsm_file", "Usage: load_twsm_file path=<filepath relative to Run/Data/>", eUsageLocation::DEV_CONSOLE, this, &Game::LoadTWSMFile );
	g_eventSystem->RegisterMethodEvent( "save_twsm_file", "Usage: save_twsm_file path=<filepath relative to Run/Data/>", eUsageLocation::DEV_CONSOLE, this, &Game::SaveTWSMFile );
	//g_eventSystem->DeRegisterObject( this );

	g_inputSystem->PushMouseOptions( CURSOR_RELATIVE, false, true );

	m_worldCamera = new Camera();
	m_worldCamera->SetColorTarget( nullptr );
	Texture* depthTexture = g_renderer->GetOrCreateDepthStencil( g_renderer->GetDefaultBackBufferSize() );
	m_worldCamera->SetDepthStencilTarget( depthTexture );

	m_worldCamera->SetOutputSize( Vec2( 16.f, 9.f ) );
	m_worldCamera->SetProjectionPerspective( 60.f, -.1f, -100.f );
	
	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	EnableDebugRendering();
	
	// Init shaders
	m_shaders.push_back( g_renderer->GetOrCreateShader( "Data/Shaders/Lit.shader" ) );
	m_shaders.push_back( g_renderer->GetOrCreateShader( "Data/Shaders/Default.shader" ) );
	m_shaders.push_back( g_renderer->GetOrCreateShader( "Data/Shaders/Normals.shader" ) );
	m_shaders.push_back( g_renderer->GetOrCreateShader( "Data/Shaders/Tangents.shader" ) );
	m_shaders.push_back( g_renderer->GetOrCreateShader( "Data/Shaders/Bitangents.shader" ) );
	m_shaders.push_back( g_renderer->GetOrCreateShader( "Data/Shaders/SurfaceNormals.shader" ) );
	
	InitializeMeshes();

	InitializeLights();

	Vec3 greyscale = Vec3( .2126f, .7152f, .0722f );
	Vec3 iBasis = greyscale.XXX();
	Vec3 jBasis = greyscale.YYY();
	Vec3 kBasis = greyscale.ZZZ();
	Mat44 colorTransform;
	colorTransform.SetBasisVectors3D( iBasis, jBasis, kBasis );

	m_colorTransformConstants.colorTransform = colorTransform;
	m_colorTransformConstants.transformPower = 0.f;
	m_colorTransformConstants.tint = Rgba8::BLACK.GetAsRGBVector();
	m_colorTransformConstants.tintPower = 0.f;
	
	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeMeshes()
{
	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	
	// Meshes
	MeshImportOptions importOptions;
	importOptions.generateNormals = true;
	importOptions.generateTangents = true;
	//importOptions.clean = true;

	AppendVertsAndIndicesForObjMeshFromFile ( vertices, indices, "Data/Models/Woman.obj", importOptions );
	m_cpuMesh = new CPUMesh( vertices, indices );
	m_gpuMesh = new GPUMesh( g_renderer, vertices, indices );
	m_meshTransform.SetPosition( Vec3( 0.f, 0.f, -2.f ) );

	// Set materials
	m_testDiffuse = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/cobblestones/cobblestonesDiffuse.png" );
	m_testNormal = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/cobblestones/cobblestonesNormal.png" );
	m_testSpecGlossEmit = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/cobblestones/cobblestonesSpecGlossEmit.png" );

	m_meshMaterial = new Material( g_renderer, "Data/Materials/Cobblestones.material"  );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeLights()
{
	m_lights[0].light.intensity = .75f;
	m_lights[0].light.color = Rgba8::WHITE.GetAsRGBVector();
	m_lights[0].light.attenuation = Vec3( 0.f, 1.f, 0.f );
	m_lights[0].light.specularAttenuation = Vec3( 0.f, 1.f, 0.f );
	m_lights[0].type = eLightType::POINT;
	m_lights[0].movementMode = eLightMovementMode::FOLLOW_CAMERA;
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );
		
	// Clean up member variables
	//PTR_SAFE_DELETE( m_defaultMaterial );
	PTR_SAFE_DELETE( m_meshMaterial );
	PTR_SAFE_DELETE( m_cpuMesh );
	PTR_SAFE_DELETE( m_gpuMesh );
	PTR_SAFE_DELETE( m_gameClock );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_debugInfoTextBox );
	PTR_SAFE_DELETE( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::RestartGame()
{
	Shutdown();
	Startup();
}


//-----------------------------------------------------------------------------------------------
void Game::Update()
{
	if ( !g_devConsole->IsOpen() ) 
	{
		UpdateFromKeyboard();
	}

	UpdateCameras();
		
	UpdateLights();

	PrintHotkeys();
	//PrintInfo();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

	UpdateCameraTransform( deltaSeconds );
	UpdateLightingCommands( deltaSeconds );
	UpdateToggleCommands( deltaSeconds );

	if ( g_inputSystem->WasKeyJustPressed( KEY_F4 ) )
	{
		g_renderer->ReloadShaders();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameraTransform( float deltaSeconds )
{
	Vec3 cameraTranslation;

	if( g_inputSystem->WasKeyJustPressed( 'O' ))
	{
		m_worldCamera->SetPosition( Vec3::ZERO );
		//m_worldCamera->SetPitchRollYawRotation( 0.f, 0.f, 0.f );
		return;
	}

	if ( g_inputSystem->IsKeyPressed( 'D' ) )
	{
		cameraTranslation.x += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'A' ) )
	{
		cameraTranslation.x -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'W' ) )
	{
		cameraTranslation.z -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		cameraTranslation.z += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'Q' ) )
	{
		cameraTranslation.y -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'E' ) )
	{
		cameraTranslation.y += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SHIFT ) )
	{
		cameraTranslation *= 10.f;
	}

	// Rotation
	Vec2 mousePosition = g_inputSystem->GetMouseDeltaPosition();
	float yaw = -mousePosition.x * s_mouseSensitivityMultiplier;
	float pitch = -mousePosition.y * s_mouseSensitivityMultiplier;
	yaw *= .009f;
	pitch *= .009f;

	Transform transform = m_worldCamera->GetTransform();
	m_worldCamera->SetPitchRollYawOrientationDegrees( transform.GetPitchDegrees() + pitch,
													  0.f,
													  transform.GetYawDegrees() + yaw );
	
	TranslateCameraFPS( cameraTranslation * deltaSeconds );

	// Update light direction
	for ( int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
	{
		GameLight& gameLight = m_lights[lightIdx];
		if ( !gameLight.isEnabled
			 || gameLight.movementMode != eLightMovementMode::FOLLOW_CAMERA )
		{
			continue;
		}
	
		SetLightDirectionToCamera( gameLight.light );

		if ( lightIdx == 0 )
		{
			m_projectionViewMatrix = m_worldCamera->GetViewMatrix();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateLightingCommands( float deltaSeconds )
{
	// Switch light
	if ( g_inputSystem->WasKeyJustPressed( KEY_RIGHTARROW ) )
	{
		m_currentLightIdx++;
		if ( m_currentLightIdx >= MAX_LIGHTS )
		{
			m_currentLightIdx = 0;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_LEFTARROW ) )
	{
		m_currentLightIdx--;
		if ( m_currentLightIdx < 0 )
		{
			m_currentLightIdx = MAX_LIGHTS - 1;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_UPARROW ) )
	{
		switch ( GetCurGameLight().type )
		{
			case eLightType::POINT:	ChangeCurrentLightType( eLightType::SPOT ); break;
			case eLightType::DIRECTIONAL: ChangeCurrentLightType( eLightType::POINT ); break;
			case eLightType::SPOT: ChangeCurrentLightType( eLightType::DIRECTIONAL ); break;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_DOWNARROW ) )
	{
		switch ( GetCurGameLight().type )
		{
			case eLightType::POINT:	ChangeCurrentLightType( eLightType::DIRECTIONAL ); break;
			case eLightType::DIRECTIONAL: ChangeCurrentLightType( eLightType::SPOT ); break;
			case eLightType::SPOT: ChangeCurrentLightType( eLightType::POINT ); break;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_ENTER ) )
	{
		GetCurGameLight().isEnabled = !GetCurGameLight().isEnabled;
	}

	// Movement
	if ( g_inputSystem->WasKeyJustPressed( KEY_F5 ) )
	{
		GetCurLight().position = Vec3::ZERO;
		GetCurGameLight().movementMode = eLightMovementMode::STATIONARY;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F6 ) )
	{
		GetCurLight().position = m_worldCamera->GetTransform().GetPosition();
		SetLightDirectionToCamera( GetCurLight() );

		GetCurGameLight().movementMode = eLightMovementMode::STATIONARY;

		m_projectionViewMatrix = m_worldCamera->GetViewMatrix();
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F7 ) )
	{
		GetCurGameLight().movementMode = eLightMovementMode::FOLLOW_CAMERA;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F8 ) )
	{
		GetCurGameLight().movementMode = eLightMovementMode::LOOP;
	}

	// Ambient Light
	if ( g_inputSystem->IsKeyPressed( '9' ) )
	{
		m_ambientIntensity -= .5f * deltaSeconds;
		m_ambientIntensity = ClampZeroToOne( m_ambientIntensity );
	}

	if ( g_inputSystem->IsKeyPressed( '0' ) )
	{
		m_ambientIntensity += .5f * deltaSeconds;
		m_ambientIntensity = ClampZeroToOne( m_ambientIntensity );
	}

	// Current light
	if ( g_inputSystem->IsKeyPressed( KEY_MINUS ) )
	{
		GetCurLight().intensity -= .5f * deltaSeconds;
		GetCurLight().intensity = ClampMin( GetCurLight().intensity, 0.f );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_PLUS ) )
	{
		GetCurLight().intensity += .5f * deltaSeconds;
		GetCurLight().intensity = ClampMin( GetCurLight().intensity, 0.f );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_LEFT_BRACKET ) )
	{
		m_colorTransformConstants.transformPower -= 1.f * deltaSeconds;
		m_colorTransformConstants.transformPower = ClampZeroToOne( m_colorTransformConstants.transformPower );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_RIGHT_BRACKET ) )
	{
		m_colorTransformConstants.transformPower += 1.f * deltaSeconds;
		m_colorTransformConstants.transformPower = ClampZeroToOne( m_colorTransformConstants.transformPower );
	}
	
	if ( g_inputSystem->IsKeyPressed( KEY_SEMICOLON ) )
	{
		m_colorTransformConstants.tintPower -= 1.f * deltaSeconds;
		m_colorTransformConstants.tintPower = ClampZeroToOne( m_colorTransformConstants.tintPower );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_QUOTE ) )
	{
		m_colorTransformConstants.tintPower += 1.f * deltaSeconds;
		m_colorTransformConstants.tintPower = ClampZeroToOne( m_colorTransformConstants.tintPower );
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_COMMA ) )
	{
		ChangeShader( m_currentShaderIdx - 1 );
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_PERIOD ) )
	{
		ChangeShader( m_currentShaderIdx + 1 );
	}

	if ( g_inputSystem->WasKeyJustPressed( 'T' ) )
	{
		if ( GetCurLight().attenuation == Vec3( 1.f, 0.f, 0.f ) )
		{
			GetCurLight().attenuation = Vec3( 0.f, 1.f, 0.f );
		}
		else if ( GetCurLight().attenuation == Vec3( 0.f, 1.f, 0.f ) )
		{
			GetCurLight().attenuation = Vec3( 0.f, 0.f, 1.f );
		}
		else if ( GetCurLight().attenuation == Vec3( 0.f, 0.f, 1.f ) )
		{
			GetCurLight().attenuation = Vec3( 1.f, 0.f, 0.f );
		}

		GetCurLight().specularAttenuation = GetCurLight().attenuation;
	}

	// Adjust global values	
	if ( g_inputSystem->IsKeyPressed( 'O' ) )
	{
		GetCurLight().halfCosOfInnerAngle += CosDegrees( 1.f ) * deltaSeconds * .05f;
		GetCurLight().halfCosOfOuterAngle += CosDegrees( 1.f ) * deltaSeconds * .05f;
		GetCurLight().halfCosOfInnerAngle = ClampMinMax( GetCurLight().halfCosOfInnerAngle, -1.f, 1.f );
		GetCurLight().halfCosOfOuterAngle = ClampMinMax( GetCurLight().halfCosOfOuterAngle, -1.f, 1.f );
	}

	if ( g_inputSystem->IsKeyPressed( 'P' ) )
	{
		GetCurLight().halfCosOfInnerAngle -= CosDegrees( 1.f ) * deltaSeconds * .05f;
		GetCurLight().halfCosOfOuterAngle -= CosDegrees( 1.f ) * deltaSeconds * .05f;
		GetCurLight().halfCosOfInnerAngle = ClampMinMax( GetCurLight().halfCosOfInnerAngle, -1.f, 1.f );
		GetCurLight().halfCosOfOuterAngle = ClampMinMax( GetCurLight().halfCosOfOuterAngle, -1.f, 1.f );
	}

	if ( g_inputSystem->IsKeyPressed( '5' ) )
	{
		m_nearFogDist -= 5.f * deltaSeconds;
		m_farFogDist -= 5.f * deltaSeconds;
		m_farFogDist = ClampMinMax( m_farFogDist, 10.f, 100.f );
		m_nearFogDist = ClampMinMax( m_nearFogDist, 0.f, m_farFogDist - 10.f );
	}
	if ( g_inputSystem->IsKeyPressed( '6' ) )
	{
		m_nearFogDist += 5.f * deltaSeconds;
		m_farFogDist += 5.f * deltaSeconds;
		m_farFogDist = ClampMinMax( m_farFogDist, 10.f, 100.f );
		m_nearFogDist = ClampMinMax( m_nearFogDist, 0.f, m_farFogDist - 10.f );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateToggleCommands( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if ( g_inputSystem->WasKeyJustPressed( 'Z' ) )
	{
		m_diffuseOn = !m_diffuseOn;

		if ( m_diffuseOn )
		{
			m_meshMaterial->SetDiffuseTexture( m_testDiffuse );
		}
		else
		{
			m_meshMaterial->SetDiffuseTexture( g_renderer->GetDefaultWhiteTexture() );
		}
	}
	
	if ( g_inputSystem->WasKeyJustPressed( 'X' ) )
	{
		m_normalOn = !m_normalOn;

		if ( m_normalOn )
		{
			m_meshMaterial->SetNormalTexture( m_testNormal );
		}
		else
		{
			m_meshMaterial->SetNormalTexture( g_renderer->GetDefaultFlatTexture() );
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( 'C' ) )
	{
		m_specGlossEmitOn = !m_specGlossEmitOn;

		if ( m_specGlossEmitOn )
		{
			m_meshMaterial->SetSpecGlossEmitTexture( m_testSpecGlossEmit );
		}
		else
		{
			m_meshMaterial->SetSpecGlossEmitTexture( g_renderer->GetDefaultSpecGlossEmissiveTexture() );
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( 'V' ) )
	{
		m_diffuseLightingOn = !m_diffuseLightingOn;

		if ( m_diffuseLightingOn )
		{
			m_diffuseEffect = 1.f;
		}
		else
		{
			m_diffuseEffect = 0.f;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( 'B' ) )
	{
		m_specularHighlightingOn = !m_specularHighlightingOn;

		if ( m_specularHighlightingOn )
		{
			m_specularEffect = 1.f;
		}
		else
		{
			m_specularEffect = 0.f;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( 'N' ) )
	{
		m_emissiveGlowOn = !m_emissiveGlowOn;

		if ( m_emissiveGlowOn )
		{
			m_emissiveEffect = 1.f;
		}
		else
		{
			m_emissiveEffect = 0.f;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( 'M' ) )
	{
		m_wireframeOn = !m_wireframeOn;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	//Rgba8 backgroundColor( 10, 10, 10, 255 );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, Rgba8::BLACK );
}


//-----------------------------------------------------------------------------------------------
void Game::TranslateCameraFPS( const Vec3& relativeTranslation )
{
	Mat44 model = m_worldCamera->GetTransform().GetAsMatrix();
	Vec3 absoluteTranslation = model.TransformVector3D( relativeTranslation );

	m_worldCamera->Translate( absoluteTranslation );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateLights()
{
	for ( int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
	{
		GameLight& gameLight = m_lights[lightIdx];
		//if ( gameLight.light.intensity == 0.f )
		if ( !gameLight.isEnabled )
		{
			continue;
		}

		switch ( gameLight.movementMode )
		{
			case eLightMovementMode::STATIONARY:
			{
				if ( gameLight.type == eLightType::POINT )
				{
					DebugAddWorldPoint( gameLight.light.position, Rgba8::GREEN );
				}
				else if ( gameLight.type == eLightType::SPOT )
				{
					DebugAddWorldArrow( gameLight.light.position, gameLight.light.position + gameLight.light.direction * .5f, Rgba8::RED );
				}
				else
				{
					DebugAddWorldArrow( gameLight.light.position, gameLight.light.position + gameLight.light.direction, Rgba8::GREEN );
				}
			} break;

			case eLightMovementMode::FOLLOW_CAMERA:
			{
				gameLight.light.position = m_worldCamera->GetTransform().GetPosition();
			} break;

			case eLightMovementMode::LOOP:
			{
				gameLight.light.position = Vec3( 0.f, 0.f, -6.f );
				gameLight.light.position.x += CosDegrees( (float)GetCurrentTimeSeconds() * 20.f ) * 8.f;
				gameLight.light.position.z += SinDegrees( (float)GetCurrentTimeSeconds() * 20.f ) * 8.f;

				if ( gameLight.type == eLightType::POINT )
				{
					DebugAddWorldPoint( gameLight.light.position, Rgba8::GREEN );
				}
				else if ( gameLight.type == eLightType::SPOT )
				{
					DebugAddWorldArrow( gameLight.light.position, gameLight.light.position + gameLight.light.direction * .5f, Rgba8::RED );
				}
				else
				{
					DebugAddWorldArrow( gameLight.light.position, gameLight.light.position + gameLight.light.direction, Rgba8::GREEN );
				}
			}
		}
	}	
}


//-----------------------------------------------------------------------------------------------
void Game::PrintHotkeys()
{
	float y = .97f;
	float lineHeight = .02f;
	float fontSize = 16.f;
	DebugAddScreenTextf( Vec4( 0.f, y, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "L,R Arrows - Current light: %d", m_currentLightIdx );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "U,D Arrows - Light type: %s", LightTypeToStr( GetCurGameLight().type ).c_str() );
	DebugAddScreenText( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "Enter - Enable/Disable light" );
	DebugAddScreenText( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::YELLOW, Rgba8::YELLOW, 0.f, "F5  - Light to origin" );
	DebugAddScreenText( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::YELLOW, Rgba8::YELLOW, 0.f, "F6  - Light to camera" );
	DebugAddScreenText( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::YELLOW, Rgba8::YELLOW, 0.f, "F7  - Light follow camera" );
	DebugAddScreenText( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::YELLOW, Rgba8::YELLOW, 0.f, "F8  - Light loop" );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "9,0 - Ambient Light intensity : %.2f", m_ambientIntensity );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "-,+ - Light intensity : %.2f", GetCurLight().intensity );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "O,P - Adjust spot light angle" );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "5,6 - Fog dist - Near: %.2f Far: %.2f", m_nearFogDist, m_farFogDist );

	y -= lineHeight;
	std::string shaderName = m_shaders[m_currentShaderIdx]->GetName();
	if ( shaderName == "Lit" )
	{
		shaderName = "Shaded";
	}
	else if ( shaderName == "Default" )
	{
		shaderName = "Fullbright";
	}
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::GREEN, Rgba8::GREEN, 0.f, "<,> - Shader : %s", shaderName.c_str() );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "Z   - Diffuse Map: %s", m_diffuseOn ? "On" : "Off" );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "X   - Normal Map: %s", m_normalOn ? "On" : "Off" );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "C   - Spec/Gloss/Emit Map: %s", m_specGlossEmitOn ? "On" : "Off" );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "V   - Diffuse Lighting: %s", m_diffuseLightingOn ? "On" : "Off" );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "B   - Specular Highlighting: %s", m_specularHighlightingOn ? "On" : "Off" );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "N   - Emissive Glow: %s", m_emissiveGlowOn ? "On" : "Off" );
	DebugAddScreenTextf( Vec4( 0.f, y -= lineHeight, 5.f, 5.f ), Vec2::ZERO, fontSize, Rgba8::WHITE, Rgba8::WHITE, 0.f, "M   - Wireframe: %s", m_wireframeOn ? "On" : "Off" );
}


//-----------------------------------------------------------------------------------------------
void Game::PrintInfo()
{
	float y = .06f;
	DebugAddScreenTextf( Vec4( 0.f, y, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "Total Texture Pool Count: %d", g_renderer->GetTotalTexturePoolCount() );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "Free Texture Pool Count: %d", g_renderer->GetTexturePoolFreeCount() );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );
	
	g_renderer->EnableFog( m_nearFogDist, m_farFogDist, Rgba8::BLACK );
	
	g_renderer->DisableAllLights();
	g_renderer->SetAmbientLight( m_ambientColor.GetAsRGBVector(), m_ambientIntensity );
	for ( int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
	{
		if ( m_lights[lightIdx].isEnabled )
		{
			g_renderer->EnableLight( lightIdx, m_lights[lightIdx].light );
		}
	}
	g_renderer->SetGamma( m_gamma );
	
	// Render model with material
	g_renderer->SetModelMatrix( m_meshTransform.GetAsMatrix() );
	g_renderer->BindMaterial( m_meshMaterial );

	g_renderer->SetDebugLightData( m_diffuseEffect, m_specularEffect, m_emissiveEffect );

	if ( m_wireframeOn )
	{
		g_renderer->SetFillMode( eFillMode::WIREFRAME );
	}
	else
	{
		g_renderer->SetFillMode( eFillMode::SOLID );
	}

	g_renderer->DrawMesh( m_gpuMesh );
		
	g_renderer->EndCamera( *m_worldCamera );
	
	// Debug Rendering
	DebugRenderWorldToCamera( m_worldCamera );
	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeShader( int nextShaderIdx )
{
	if ( nextShaderIdx > (int)m_shaders.size() - 1 )
	{
		nextShaderIdx = 0;
	}
	else if ( nextShaderIdx < 0 )
	{
		nextShaderIdx = (int)m_shaders.size() - 1;
	}

	m_currentShaderIdx = nextShaderIdx;
	m_meshMaterial->SetShader( m_shaders[m_currentShaderIdx] );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeCurrentLightType( eLightType newLightype )
{
	switch ( newLightype )
	{
		case eLightType::POINT:
		{
			GetCurLight().isDirectional = 0.f;
			GetCurLight().halfCosOfInnerAngle = 0.f;
			GetCurLight().halfCosOfOuterAngle = 0.f;
		} break;

		case eLightType::DIRECTIONAL:
		{
			GetCurLight().isDirectional = 1.f;
			GetCurLight().halfCosOfInnerAngle = 0.f;
			GetCurLight().halfCosOfOuterAngle = 0.f;
		} break;

		case eLightType::SPOT:
		{
			GetCurLight().isDirectional = 0.f;
			GetCurLight().halfCosOfInnerAngle = CosDegrees( 5.f );
			GetCurLight().halfCosOfOuterAngle = CosDegrees( 10.f );
		} break;
	}

	GetCurGameLight().type = newLightype;
}


//-----------------------------------------------------------------------------------------------
std::string Game::LightTypeToStr( eLightType lightType )
{
	switch ( lightType )
	{
		case eLightType::POINT:	return "Point";
		case eLightType::DIRECTIONAL: return "Directional";
		case eLightType::SPOT: return "Spot";
	}

	return "Invalid";
}


//-----------------------------------------------------------------------------------------------
void Game::SetLightDirectionToCamera( Light& light )
{
	Mat44 model = m_worldCamera->GetTransform().GetAsMatrix();
	Vec3 cameraForwardDir = model.TransformVector3D( Vec3( 0.f, 0.f, -1.f ) ).GetNormalized();
	light.direction = cameraForwardDir;
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets...", Rgba8::WHITE );

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
bool Game::SetMouseSensitivity( EventArgs* args )
{
	s_mouseSensitivityMultiplier = args->GetValue( "multiplier", 1.f );

	return false;
}


//-----------------------------------------------------------------------------------------------
void Game::UnSubscribeGameMethods( EventArgs* args )
{
	UNUSED( args );

	g_eventSystem->DeRegisterObject( this );
}


//-----------------------------------------------------------------------------------------------
void Game::SetAmbientLightColor( EventArgs* args )
{
	Vec3 color = args->GetValue( "color", Vec3( 1.f, 1.f, 1.f ) );

	m_ambientColor.SetFromNormalizedVector( Vec4( color, 1.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::SetPointLightColor( EventArgs* args )
{
	GetCurLight().color = args->GetValue( "color", Vec3( 1.f, 1.f, 1.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadObjFile( EventArgs* args )
{
	std::string modelPath = args->GetValue( "path", "" );

	if ( modelPath.empty() )
	{
		g_devConsole->PrintError( "Must specify path of file to load (relative to Run/Data)" );
		return;
	}

	std::string fileExtension = GetFileExtension( modelPath );
	if ( fileExtension.empty() )
	{
		modelPath.append( ".obj" );
	}
	else if ( IsEqualIgnoreCase( fileExtension, ".obj" ) )
	{
		// all good, don't need to change filename
	}
	else
	{
		g_devConsole->PrintError( "Can only load files with extension: .obj" );
		return;
	}
	
	MeshImportOptions importOptions;
	importOptions.generateNormals = true;
	importOptions.generateTangents = true;
	//importOptions.clean = true;

	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	AppendVertsAndIndicesForObjMeshFromFile( vertices, indices, "Data/" + modelPath, importOptions );
	
	PTR_SAFE_DELETE( m_cpuMesh );
	PTR_SAFE_DELETE( m_gpuMesh );

	m_cpuMesh = new CPUMesh( vertices, indices );
	m_gpuMesh = new GPUMesh( g_renderer, vertices, indices );
	m_meshTransform.SetPosition( Vec3( 0.f, 0.f, -2.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadTWSMFile( EventArgs* args )
{
	std::string modelPath = args->GetValue( "path", "" );

	if ( modelPath.empty() )
	{
		g_devConsole->PrintError( "Must specify path of file to load (relative to Run/Data)" );
		return;
	}

	std::string fileExtension = GetFileExtension( modelPath );
	if ( fileExtension.empty() )
	{
		modelPath.append( ".twsm" );
	}
	else if ( IsEqualIgnoreCase( fileExtension, ".twsm" ) )
	{
		// all good, don't need to change filename
	}
	else
	{
		g_devConsole->PrintError( "Can only save twsm files with extension: .twsm" );
		return;
	}

	CPUMesh* newMesh = LoadTWSMFileIntoCPUMesh( "Data/" + modelPath );
	if ( newMesh == nullptr )
	{
		return;
	}

	PTR_SAFE_DELETE( m_cpuMesh );
	PTR_SAFE_DELETE( m_gpuMesh );
	
	m_cpuMesh = newMesh;
	m_gpuMesh = new GPUMesh( g_renderer, *m_cpuMesh );
}


//-----------------------------------------------------------------------------------------------
void Game::SaveTWSMFile( EventArgs* args )
{
	std::string modelPath = args->GetValue( "path", "" );

	if ( modelPath.empty() )
	{
		g_devConsole->PrintError( "Must specify path of file to load (relative to Run/Data)" );
		return;
	}

	std::string fileExtension = GetFileExtension( modelPath );
	if ( fileExtension.empty() )
	{
		modelPath.append( ".twsm" );
	}
	else if ( IsEqualIgnoreCase( fileExtension, ".twsm" ) )
	{
		// all good, don't need to change filename
	}
	else
	{
		g_devConsole->PrintError( "Can only save twsm files with extension: .twsm" );
		return;
	}

	SaveMeshAsTWSMFile( *m_cpuMesh, "Data/" + modelPath );
}

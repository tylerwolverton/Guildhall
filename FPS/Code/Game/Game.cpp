#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/TextBox.hpp"
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
#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"


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
	g_eventSystem->RegisterEvent( "set_mouse_sensitivity", "Usage: set_mouse_sensitivity multiplier=NUMBER. Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );
	g_eventSystem->RegisterMethodEvent( "light_set_ambient_color", "Usage: light_set_ambient_color color=r,g,b", eUsageLocation::DEV_CONSOLE, this, &Game::SetAmbientLightColor );
	g_eventSystem->RegisterMethodEvent( "light_set_color", "Usage: light_set_color color=r,g,b", eUsageLocation::DEV_CONSOLE, this, &Game::SetPointLightColor );
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

	// For testing SetShaderByName
	g_renderer->GetOrCreateShader( "Data/Shaders/Fresnel.shader" );
	m_fresnelData.color = Rgba8::GREEN.GetAsRGBVector();
	m_fresnelData.power = 32.f;

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
	// Cubes
	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	AppendVertsAndIndicesForCubeMesh( vertices, indices, Vec3::ZERO, 2.f, Rgba8::WHITE );

	m_cubeMesh = new GPUMesh( g_renderer, vertices, indices );

	m_cubeMeshTransform.SetPosition( Vec3( -5.f, 0.f, -6.f ) );
	m_cubeMeshTransformDissolve.SetPosition( Vec3( -5.f, 0.f, -6.f ) );

	// Quad
	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForQuad( vertices, indices, AABB2( -1.f, -1.f, 1.f, 1.f ), Rgba8::WHITE );

	m_quadMesh = new GPUMesh( g_renderer, vertices, indices );

	m_quadMeshTransform.SetPosition( Vec3( 0.f, 0.f, -6.f ) );

	// Spheres
	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForSphereMesh( vertices, indices, Vec3::ZERO, 1.f, 64, 64, Rgba8::WHITE );

	m_sphereMesh = new GPUMesh( g_renderer, vertices, indices );

	m_sphereMeshTransform.SetPosition( Vec3( 5.f, 0.f, -6.f ) );
	m_sphereMeshFresnelTransform.SetPosition( Vec3( 0.f, 0.f, -6.f ) );
	m_sphereMeshTriplanarTransform.SetPosition( Vec3( 5.f, 0.f, -6.f ) );

	// Meshes
	vertices.clear();
	indices.clear();
	MeshImportOptions importOptions;
	importOptions.generateNormals = true;
	importOptions.generateTangents = true;
	importOptions.transform = Mat44::CreateTranslation3D( Vec3( -5.f, 0.f, -0.f ) );
	importOptions.transform.PushTransform( Mat44::CreateUniformScale3D( .05f ) );
	AppendVertsForObjMeshFromFile ( vertices, "Data/Meshes/teapot.obj", importOptions );
	m_teapotMesh = new GPUMesh( g_renderer, vertices, indices );

	vertices.clear();
	indices.clear();
	importOptions.transform = Mat44::CreateUniformScale3D( .5f );
	//importOptions.clean = true;
	AppendVertsForObjMeshFromFile ( vertices, "Data/Models/Vespa/Vespa.obj", importOptions );
	//AppendVertsForObjMeshFromFile( vertices, "Data/Models/scifi_fighter/mesh.obj", importOptions );
	//AppendVertsAndIndicesForObjMeshFromFile( vertices, indices, "Data/Models/scifi_fighter/mesh.obj", importOptions );
	m_objMesh = new GPUMesh( g_renderer, vertices, indices );
	m_objMeshTransform.SetPosition( Vec3( 0.f, 0.f, -2.f ) );

	// Set materials
	m_defaultMaterial = new Material( g_renderer, "Data/Materials/Default.material" );
	m_teapotMaterial = new Material( g_renderer, "Data/Materials/Teapot.material" );
	//m_objMaterial = new Material( g_renderer, "Data/Models/scifi_fighter/scifi_fighter.material" );
	m_objMaterial = new Material( g_renderer, "Data/Models/Vespa/Vespa.material" );

	m_fresnelMaterial = new Material( g_renderer, "Data/Materials/Default.material" );
	m_fresnelMaterial->SetShader( g_renderer->GetShaderByName( "Fresnel" ) );
	m_dissolveMaterial = new Material( g_renderer, "Data/Materials/Dissolve.material" );
	m_triplanarMaterial = new Material( g_renderer, "Data/Materials/Triplanar.material" );
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

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
	PTR_SAFE_DELETE( m_defaultMaterial );
	PTR_SAFE_DELETE( m_objMaterial );
	PTR_SAFE_DELETE( m_teapotMaterial );
	PTR_SAFE_DELETE( m_fresnelMaterial );
	PTR_SAFE_DELETE( m_dissolveMaterial );
	PTR_SAFE_DELETE( m_triplanarMaterial );
	PTR_SAFE_DELETE( m_teapotMesh );
	PTR_SAFE_DELETE( m_objMesh );
	PTR_SAFE_DELETE( m_quadMesh );
	PTR_SAFE_DELETE( m_cubeMesh );
	PTR_SAFE_DELETE( m_sphereMesh );
	PTR_SAFE_DELETE( m_world );
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
		
	m_fresnelMaterial->SetData( m_fresnelData );

	DissolveConstants dissolveData;// = m_dissolveMaterial->GetDataAs<DissolveConstants>();
	dissolveData.dissolveFactor = m_dissolveFactor;
	dissolveData.edgeWidth = m_dissolveEdge;
	dissolveData.startColor = Rgba8::RED.GetAsRGBVector();
	dissolveData.endColor = Rgba8::BLUE.GetAsRGBVector();
	m_dissolveMaterial->SetData( dissolveData );

	UpdateLights();

	PrintHotkeys();
	PrintDiageticHotkeys();
	PrintInfo();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

	UpdateCameraTransform( deltaSeconds );
	UpdateDebugDrawCommands();
	UpdateLightingCommands( deltaSeconds );

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
		m_worldCamera->SetPitchRollYawRotation( 0.f, 0.f, 0.f );
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

	if ( g_inputSystem->IsKeyPressed( 'C' ) )
	{
		cameraTranslation.y -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SPACEBAR ) )
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
	m_worldCamera->SetPitchRollYawRotation( transform.m_orientation.x + pitch,
											0.f,
											transform.m_orientation.z + yaw );

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

	// Translation
	TranslateCameraFPS( cameraTranslation * deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateDebugDrawCommands()
{
	// Debug Commands
	if ( g_inputSystem->IsKeyPressed( 'Q' ) )
	{
		DebugAddWorldPoint( m_worldCamera->GetTransform().GetPosition(), .01f, Rgba8::GREEN, Rgba8::RED, 10.f, DEBUG_RENDER_XRAY );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'E' ) )
	{
		DebugAddWorldArrow( m_worldCamera->GetTransform().GetPosition(), Rgba8::RED, Rgba8::GREEN,
							m_cubeMeshTransform.GetPosition(), Rgba8::WHITE, Rgba8::BLACK,
							10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'R' ) )
	{
		AABB3 box( m_worldCamera->GetTransform().GetPosition(), m_worldCamera->GetTransform().GetPosition() + Vec3::ONE );
		DebugAddWorldWireBounds( box, Rgba8::YELLOW, 10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'Y' ) )
	{
		DebugAddWorldWireSphere( m_worldCamera->GetTransform().GetPosition(), 2.f, Rgba8::WHITE, Rgba8::BLUE, 10.f, DEBUG_RENDER_ALWAYS );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'U' ) )
	{
		Vec3 p0 = m_worldCamera->GetTransform().GetPosition();
		Vec3 p1 = p0 + Vec3( 10.f, 0.f, 0.f );
		Vec3 p2 = p0 + Vec3( 0.f, 10.f, 0.f );
		Vec3 p3 = p0 + Vec3( 10.f, 10.f, 0.f );
		DebugAddWorldQuad( p0, p1, p2, p3, AABB2::ONE_BY_ONE, Rgba8::GREEN, Rgba8::RED, 20.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'I' ) )
	{
		DebugAddWorldBasis( m_worldCamera->GetTransform().GetAsMatrix(), Rgba8::WHITE, Rgba8::BLACK, 10.f );
	}

	if ( g_inputSystem->WasKeyJustPressed( '1' ) )
	{
		DebugAddScreenPoint( Vec2( 1920.f, 1080.f ) * .5f, 4.f, Rgba8::GREEN, Rgba8::RED, 5.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '2' ) )
	{
		DebugAddScreenLine( Vec2::ZERO, Rgba8::BLUE, Vec2( 1920.f, 1080.f ) * .5f, Rgba8::WHITE, Rgba8::BLACK, Rgba8::YELLOW, 3.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '3' ) )
	{
		DebugAddScreenArrow( Vec2( 250.f, 300.f ), Rgba8::BLUE, Vec2( 1000.f, 12.f ), Rgba8::GREEN, Rgba8::RED, Rgba8::YELLOW, 3.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '4' ) )
	{
		DebugAddScreenQuad( AABB2( Vec2( 250.f, 12.f ), Vec2( 1000.f, 120.f ) ), Rgba8::GREEN, Rgba8::RED, 10.f );
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

	if ( g_inputSystem->IsKeyPressed( '7' ) )
	{
		m_fresnelData.power -= 5.f * deltaSeconds;
		m_fresnelData.power = ClampMin( m_fresnelData.power, 0.f );
	}

	if ( g_inputSystem->IsKeyPressed( '8' ) )
	{
		m_fresnelData.power += 5.f * deltaSeconds;
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
	if ( g_inputSystem->IsKeyPressed( 'G' ) )
	{
		m_gamma -= 1.f * deltaSeconds;
		m_gamma = ClampMin( m_gamma, 1.f );
	}

	if ( g_inputSystem->IsKeyPressed( 'H' ) )
	{
		m_gamma += 1.f * deltaSeconds;
		m_gamma = ClampMin( m_gamma, 1.f );
	}

	if ( g_inputSystem->IsKeyPressed( 'N' ) )
	{
		m_dissolveFactor -= 1.f * deltaSeconds;
		m_dissolveFactor = ClampZeroToOne( m_dissolveFactor );
	}

	if ( g_inputSystem->IsKeyPressed( 'M' ) )
	{
		m_dissolveFactor += 1.f * deltaSeconds;
		m_dissolveFactor = ClampZeroToOne( m_dissolveFactor );
	}

	if ( g_inputSystem->IsKeyPressed( 'J' ) )
	{
		m_dissolveEdge -= 1.f * deltaSeconds;
		m_dissolveEdge = ClampZeroToOne( m_dissolveEdge );
	}

	if ( g_inputSystem->IsKeyPressed( 'K' ) )
	{
		m_dissolveEdge += 1.f * deltaSeconds;
		m_dissolveEdge = ClampZeroToOne( m_dissolveEdge );
	}

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

	if ( g_inputSystem->WasKeyJustPressed( 'B' ) )
	{
		m_bloomEnabled = !m_bloomEnabled;
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
				gameLight.light.position = m_quadMeshTransform.GetPosition();
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
	DebugAddScreenTextf( Vec4( 0.f, y, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "L,R Arrows - Current light: %d", m_currentLightIdx );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "U,D Arrows - Light type: %s", LightTypeToStr( GetCurGameLight().type ).c_str() );
	DebugAddScreenText( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "Enter - Enable/Disable light" );
	DebugAddScreenText( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "F5  - Light to origin" );
	DebugAddScreenText( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "F6  - Light to camera" );
	DebugAddScreenText( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "F7  - Light follow camera" );
	DebugAddScreenText( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "F8  - Light loop" );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "9,0 - Ambient Light intensity : %.2f", m_ambientIntensity );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "-,+ - Light intensity : %.2f", GetCurLight().intensity );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "t   - Attenuation : ( %.2f, %.2f, %.2f )", GetCurLight().attenuation.x, GetCurLight().attenuation.y, GetCurLight().attenuation.z );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "O,P - Adjust spot light angle" );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "[,] - Greyscale power : %.2f", m_colorTransformConstants.transformPower );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, ";,' - Tint power : %.2f", m_colorTransformConstants.tintPower );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "B' -  Bloom : %s", m_bloomEnabled ? "enabled" : "disabled" );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "G,H - Gamma : %.2f", m_gamma );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "5,6 - Fog dist - Near: %.2f Far: %.2f", m_nearFogDist, m_farFogDist );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "<,> - Shader : %s", m_shaders[m_currentShaderIdx]->GetName().c_str() );
}


//-----------------------------------------------------------------------------------------------
void Game::PrintDiageticHotkeys()
{
	// Dissolve
	DebugAddWorldBillboardTextf( m_cubeMeshTransformDissolve.m_position + Vec3( 0.f, 2.f, 0.f ), Vec2( .5f, .5f ), Rgba8::WHITE, 0.f, eDebugRenderMode::DEBUG_RENDER_USE_DEPTH, "Dissolve Shader" );
	DebugAddWorldBillboardTextf( m_cubeMeshTransformDissolve.m_position + Vec3( 0.f, 1.75f, 0.f ), Vec2( .5f, .5f ), Rgba8::WHITE, 0.f, eDebugRenderMode::DEBUG_RENDER_USE_DEPTH, "N,M - intensity : %.2f", m_dissolveFactor );
	DebugAddWorldBillboardTextf( m_cubeMeshTransformDissolve.m_position + Vec3( 0.f, 1.5f, 0.f ), Vec2( .5f, .5f ), Rgba8::WHITE, 0.f, eDebugRenderMode::DEBUG_RENDER_USE_DEPTH, "J.K - edge size : %.2f", m_dissolveEdge );

	// Fresnel
	DebugAddWorldBillboardTextf( m_sphereMeshFresnelTransform.m_position + Vec3( 0.f, 2.f, 0.f ), Vec2( .5f, .5f ), Rgba8::WHITE, 0.f, eDebugRenderMode::DEBUG_RENDER_USE_DEPTH, "Fresnel Shader" );
	DebugAddWorldBillboardTextf( m_sphereMeshFresnelTransform.m_position + Vec3( 0.f, 1.75f, 0.f ), Vec2( .5f, .5f ), Rgba8::WHITE, 0.f, eDebugRenderMode::DEBUG_RENDER_USE_DEPTH, "7,8 - power : %.2f", m_fresnelData.power );
	
	// Triplanar
	DebugAddWorldBillboardTextf( m_sphereMeshTriplanarTransform.m_position + Vec3( 0.f, 2.f, 0.f ), Vec2( .5f, .5f ), Rgba8::WHITE, 0.f, eDebugRenderMode::DEBUG_RENDER_USE_DEPTH, "Triplanar Shader" );
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
	Texture* backbuffer = g_renderer->GetBackBuffer();
	Texture* colorTarget = g_renderer->AcquireRenderTargetMatching( backbuffer );
	Texture* bloomTarget = g_renderer->AcquireRenderTargetMatching( backbuffer );

	m_worldCamera->SetColorTarget( 0, colorTarget );
	m_worldCamera->SetColorTarget( 1, bloomTarget );

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
	
	// Render test teapot
	g_renderer->SetModelMatrix( Mat44::IDENTITY );
	g_renderer->BindMaterial( m_teapotMaterial );

	g_renderer->DrawMesh( m_teapotMesh );

	// Render obj with material
	g_renderer->SetModelMatrix( m_objMeshTransform.GetAsMatrix() );
	g_renderer->BindMaterial( m_objMaterial );
	g_renderer->DrawMesh( m_objMesh );

	// Fresnel
	g_renderer->SetModelMatrix( m_sphereMeshFresnelTransform.GetAsMatrix() );
	g_renderer->BindMaterial( m_defaultMaterial );
	g_renderer->DrawMesh( m_sphereMesh );
	
	g_renderer->SetModelMatrix( m_sphereMeshFresnelTransform.GetAsMatrix() );
	g_renderer->BindMaterial( m_fresnelMaterial );
	g_renderer->DrawMesh( m_sphereMesh );
	   
	// Dissolve
	g_renderer->SetModelMatrix( m_cubeMeshTransformDissolve.GetAsMatrix() );
	g_renderer->BindMaterial( m_dissolveMaterial );
	g_renderer->DrawMesh( m_cubeMesh );

	// Triplanar
	g_renderer->SetModelMatrix( m_sphereMeshTriplanarTransform.GetAsMatrix() );
	g_renderer->BindMaterial( m_triplanarMaterial );
	g_renderer->DrawMesh( m_sphereMesh );

	g_renderer->EndCamera( *m_worldCamera );

	// Render full screen effect
	ShaderProgram* shaderProg = g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/ImageEffectColorTransform.hlsl" );
	g_renderer->SetMaterialData( (void*)&m_colorTransformConstants, sizeof( m_colorTransformConstants ) );

	Texture* colorTransformTarget = g_renderer->AcquireRenderTargetMatching( backbuffer );

	g_renderer->StartEffect( colorTransformTarget, colorTarget, shaderProg );
	g_renderer->EndEffect();

	if ( m_bloomEnabled )
	{
		shaderProg = g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/ImageEffectBloom.hlsl" );

		Material mat( g_renderer );
		Shader shader;
		shader.SetShaderProgram( shaderProg );

		mat.SetShader( &shader );
		mat.SetUserTexture( 0, colorTransformTarget );

		g_renderer->StartEffect( backbuffer, bloomTarget, &mat );
		g_renderer->EndEffect();
	}
	else
	{
		g_renderer->CopyTexture( backbuffer, colorTransformTarget );
	}

	m_worldCamera->SetColorTarget( backbuffer );

	g_renderer->ReleaseRenderTarget( colorTarget );
	g_renderer->ReleaseRenderTarget( bloomTarget );
	g_renderer->ReleaseRenderTarget( colorTransformTarget );

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
	m_objMaterial->SetShader( m_shaders[m_currentShaderIdx] );
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
void Game::LoadNewMap( const std::string& mapName )
{
	PTR_SAFE_DELETE( m_world );

	m_world = new World();
	m_world->BuildNewMap( mapName );
}


//-----------------------------------------------------------------------------------------------
void Game::AddScreenShakeIntensity(float intensity)
{
	m_screenShakeIntensity += intensity;
}


//-----------------------------------------------------------------------------------------------
void Game::PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines )
{
	if ( (int)textLines.size() == 0 )
	{
		return;
	}

	m_debugInfoTextBox->SetText( textLines[0], color );

	for ( int textLineIndex = 1; textLineIndex < (int)textLines.size(); ++textLineIndex )
	{
		m_debugInfoTextBox->AddLineOFText( textLines[ textLineIndex ], color );
	}
}


//-----------------------------------------------------------------------------------------------
bool Game::SetMouseSensitivity( EventArgs* args )
{
	s_mouseSensitivityMultiplier = args->GetValue( "multiplier", 1.f );

	return false;
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

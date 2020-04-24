#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
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
static Vec3 s_ambientLightColor = Vec3( 1.f, 1.f, 1.f );
static Vec3 s_currentLightColor = Vec3( 1.f, 1.f, 1.f);


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
	std::string testStr( "1//1 " );
	Strings trimmed = SplitStringOnDelimiter( testStr, '/' );


	g_eventSystem->RegisterEvent( "set_mouse_sensitivity", "Usage: set_mouse_sensitivity multiplier=NUMBER. Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );
	g_eventSystem->RegisterEvent( "light_set_ambient_color", "Usage: light_set_ambient_color color=r,g,b", eUsageLocation::DEV_CONSOLE, SetAmbientLightColor );
	g_eventSystem->RegisterEvent( "light_set_color", "Usage: light_set_color color=r,g,b", eUsageLocation::DEV_CONSOLE, SetPointLightColor );

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
	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	EnableDebugRendering();

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );

	m_fresnelData.color = Rgba8::GREEN.GetAsRGBVector();
	m_fresnelData.power = 32.f;

	// Init shaders
	m_shaderPaths.push_back( "Data/Shaders/Lit.hlsl" );
	m_shaderNames.push_back( "Lit" );
	m_shaderPaths.push_back( "Data/Shaders/Default.hlsl" );
	m_shaderNames.push_back( "Default" );
	m_shaderPaths.push_back( "Data/Shaders/Normals.hlsl" );
	m_shaderNames.push_back( "Normals" );
	m_shaderPaths.push_back( "Data/Shaders/Tangents.hlsl" );
	m_shaderNames.push_back( "Tangents" );
	m_shaderPaths.push_back( "Data/Shaders/Bitangents.hlsl" );
	m_shaderNames.push_back( "Bitangents" );
	m_shaderPaths.push_back( "Data/Shaders/SurfaceNormals.hlsl" );
	m_shaderNames.push_back( "Surface Normals" );

	InitializeMeshes();

	InitializeLights();
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
	m_cubeMeshTransformDissolve.SetPosition( Vec3( -5.f, 0.f, 6.f ) );

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
	m_sphereMeshFresnelTransform.SetPosition( Vec3( 0.f, 0.f, 6.f ) );
	m_sphereMeshTriplanarTransform.SetPosition( Vec3( 5.f, 0.f, 6.f ) );

	// Move to RenderContext::LoadMeshFromFile
	OBJLoader loader;
	m_teapotMesh = loader.LoadFromFile( g_renderer, "Data/Meshes/vespa_final.obj" );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeLights()
{
	// Finite spot
	m_lights[0].light.intensity = .5f;
	m_lights[0].light.color = Rgba8::WHITE.GetAsRGBVector();
	m_lights[0].light.attenuation = Vec3( 0.f, 1.f, 0.f );
	m_lights[0].light.specularAttenuation = Vec3( 0.f, 1.f, 0.f );
	/*m_lights[0].light.halfCosOfInnerAngle = CosDegrees( 7.f );
	m_lights[0].light.halfCosOfOuterAngle = CosDegrees( 15.f );*/
	m_lights[0].type = eLightType::POINT;
	m_lights[0].movementMode = eLightMovementMode::FOLLOW_CAMERA;

	//// Finite point
	//m_lights[1].light.intensity = .3f;
	//m_lights[1].light.color = Rgba8::RED.GetAsRGBVector();
	//m_lights[1].light.attenuation = Vec3( 0.f, 1.f, 0.f );
	//m_lights[1].light.specularAttenuation = Vec3( 0.f, 1.f, 0.f );
	//m_lights[1].light.position = m_quadMeshTransform.GetPosition() + Vec3( -.5f, -.5f, .5f );
	//m_lights[1].type = eLightType::POINT;
	//m_lights[1].movementMode = eLightMovementMode::STATIONARY;

	//// Infinite point
	//m_lights[2].light.intensity = .4f;
	//m_lights[2].light.color = Rgba8::BLUE.GetAsRGBVector();
	//m_lights[2].light.attenuation = Vec3( 1.f, 0.f, 0.f );
	//m_lights[2].light.specularAttenuation = Vec3( 1.f, 0.f, 0.f );
	//m_lights[2].light.position = m_quadMeshTransform.GetPosition() + Vec3( .5f, -.5f, .5f );
	//m_lights[2].type = eLightType::POINT;
	//m_lights[2].movementMode = eLightMovementMode::STATIONARY;
	//
	//// Infinite Directional
	//m_lights[3].light.intensity = .5f;
	//m_lights[3].light.color = Rgba8::WHITE.GetAsRGBVector();
	//m_lights[3].light.attenuation = Vec3( 1.f, 0.f, 0.f );
	//m_lights[3].light.specularAttenuation = Vec3( 1.f, 0.f, 0.f );
	//m_lights[3].light.position = Vec3( -5.f, 3.f, -10.f );
	//m_lights[3].light.direction = Vec3( 0.f, -1.f, 1.f ).GetNormalized();
	//m_lights[3].type = eLightType::DIRECTIONAL;
	//m_lights[3].movementMode = eLightMovementMode::STATIONARY;

	//// Infinite spot
	//m_lights[4].light.intensity = .2f;
	//m_lights[4].light.color = Rgba8::PURPLE.GetAsRGBVector();
	//m_lights[4].light.attenuation = Vec3( 1.f, 0.f, 0.f );
	//m_lights[4].light.specularAttenuation = Vec3( 1.f, 0.f, 0.f );
	//m_lights[4].light.position = m_sphereMeshTransform.GetPosition() + Vec3( 1.5f, -2.5f, 1.f );
	//m_lights[4].light.direction = Vec3( -.5f, 1.f, -1.f ).GetNormalized();
	//m_lights[4].light.halfCosOfInnerAngle = CosDegrees( 10.f );
	//m_lights[4].light.halfCosOfOuterAngle = CosDegrees( 15.f );
	//m_lights[4].type = eLightType::SPOT;
	//m_lights[4].movementMode = eLightMovementMode::STATIONARY;
	//
	//// Finite point
	//m_lights[5].light.intensity = .3f;
	//m_lights[5].light.color = Rgba8::GREEN.GetAsRGBVector();
	//m_lights[5].light.attenuation = Vec3( 0.f, 1.f, 0.f );
	//m_lights[5].light.specularAttenuation = Vec3( 0.f, 1.f, 0.f );
	//m_lights[5].light.position = m_quadMeshTransform.GetPosition() + Vec3( -.5f, .5f, .5f );
	//m_lights[5].type = eLightType::POINT;
	//m_lights[5].movementMode = eLightMovementMode::STATIONARY;

	//// Finite point
	//m_lights[6].light.intensity = .3f;
	//m_lights[6].light.color = Rgba8::YELLOW.GetAsRGBVector();
	//m_lights[6].light.attenuation = Vec3( 0.f, 1.f, 0.f );
	//m_lights[6].light.specularAttenuation = Vec3( 0.f, 1.f, 0.f );
	//m_lights[6].light.position = m_quadMeshTransform.GetPosition() + Vec3( .5f, .5f, .5f );
	//m_lights[6].type = eLightType::POINT;
	//m_lights[6].movementMode = eLightMovementMode::STATIONARY;

	//// Rotating point
	//m_lights[7].light.intensity = .3f;
	//m_lights[7].light.color = Rgba8::ORANGE.GetAsRGBVector();
	//m_lights[7].light.attenuation = Vec3( 0.f, 1.f, 0.f );
	//m_lights[7].light.specularAttenuation = Vec3( 0.f, 1.f, 0.f );
	//m_lights[7].type = eLightType::POINT;
	//m_lights[7].movementMode = eLightMovementMode::LOOP;
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
	PTR_SAFE_DELETE( m_teapotMesh );
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

	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();
	m_cubeMeshTransform.RotatePitchRollYawDegrees( deltaSeconds * 15.f, 0.f, deltaSeconds * 35.f );
	m_sphereMeshTransform.RotatePitchRollYawDegrees( deltaSeconds * 35.f, 0.f, -deltaSeconds * 20.f );
 
	UpdateLights();

	//PrintHotkeys();
	//PrintDiageticHotkeys();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

	UpdateCameraTransform( deltaSeconds );
	UpdateDebugDrawCommands();
	UpdateLightingCommands( deltaSeconds );

	if ( g_inputSystem->WasKeyJustPressed( KEY_F2 ) )
	{
		g_renderer->CycleSampler();
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
	{
		g_renderer->CycleBlendMode();
	}
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
		cameraTranslation.y += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SPACEBAR ) )
	{
		cameraTranslation.y -= 1.f;
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
	if ( g_inputSystem->WasKeyJustPressed( 'B' ) )
	{
		DebugAddWorldBillboardText( m_worldCamera->GetTransform().GetPosition() - m_worldCamera->GetTransform().GetAsMatrix().GetKBasis3D() * 5.f, Vec2::ONE, Rgba8::GREEN, Rgba8::RED, 35.f, eDebugRenderMode::DEBUG_RENDER_XRAY, "Mid!" );
		DebugAddWorldBillboardTextf( m_worldCamera->GetTransform().GetPosition() - m_worldCamera->GetTransform().GetAsMatrix().GetKBasis3D() * 5.f, Vec2::ZERO, Rgba8::GREEN, 35.f, eDebugRenderMode::DEBUG_RENDER_XRAY, "%d!", 15 );
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
		
		s_currentLightColor = GetCurLight().color;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_LEFTARROW ) )
	{
		m_currentLightIdx--;
		if ( m_currentLightIdx < 0 )
		{
			m_currentLightIdx = MAX_LIGHTS - 1;
		}
		
		s_currentLightColor = GetCurLight().color;
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
		GetCurLight().intensity = ClampZeroToOne( GetCurLight().intensity );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_PLUS ) )
	{
		GetCurLight().intensity += .5f * deltaSeconds;
		GetCurLight().intensity = ClampZeroToOne( GetCurLight().intensity );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_LEFT_BRACKET ) )
	{
		m_specularFactor -= .5f * deltaSeconds;
		m_specularFactor = ClampZeroToOne( m_specularFactor );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_RIGHT_BRACKET ) )
	{
		m_specularFactor += .5f * deltaSeconds;
		m_specularFactor = ClampZeroToOne( m_specularFactor );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SEMICOLON ) )
	{
		m_specularPower -= 5.f * deltaSeconds;
		m_specularPower = ClampMin( m_specularPower, 1.f );
	}

	if ( g_inputSystem->IsKeyPressed( KEY_QUOTE ) )
	{
		m_specularPower += 5.f * deltaSeconds;
		m_specularPower = ClampMin( m_specularPower, 1.f );
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
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	Rgba8 backgroundColor( 10, 10, 10, 255 );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, backgroundColor );
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
	
	GetCurLight().color = s_currentLightColor;
	
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
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "[,] - Specular factor : %.2f", m_specularFactor );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, ";,' - Specular power : %.2f", m_specularPower );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "G,H - Gamma : %.2f", m_gamma );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "5,6 - Fog dist - Near: %.2f Far: %.2f", m_nearFogDist, m_farFogDist );
	DebugAddScreenTextf( Vec4( 0.f, y -= .03f, 5.f, 5.f ), Vec2::ZERO, 20.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, "<,> - Shader : %s", m_shaderNames[m_currentShaderIdx].c_str() );
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
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	g_renderer->EnableFog( m_nearFogDist, m_farFogDist, Rgba8::BLACK );
	g_renderer->BindDiffuseTexture( nullptr );
	//g_renderer->BindNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/brick_normal.png" ) );

	g_renderer->BindShader( m_shaderPaths[m_currentShaderIdx].c_str() );
	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );
	
	g_renderer->DisableAllLights();
	g_renderer->SetAmbientLight( s_ambientLightColor, m_ambientIntensity );
	for ( int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
	{
		if ( m_lights[lightIdx].isEnabled )
		{
			g_renderer->EnableLight( lightIdx, m_lights[lightIdx].light );
		}
	}
	g_renderer->SetGamma( m_gamma );
	
	// Render normal objects
	//Mat44 model = m_cubeMeshTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_cubeMesh );

	//model = m_quadMeshTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_quadMesh );
	//
	//model = m_sphereMeshTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_sphereMesh );


	//g_renderer->BindDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/grass_d.png" ) );
	//g_renderer->BindDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/mask_head_d.png" ) );
	g_renderer->BindDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/vespa_d_4k.png" ) );
	g_renderer->BindNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/vespa_n_4k.png" ) );
	//g_renderer->BindNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/brick_normal.png" ) );
	Mat44 model = Mat44();
	g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_renderer->DrawMesh( m_teapotMesh );

	//// Fresnel
	//model = m_sphereMeshFresnelTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_sphereMesh );

	//g_renderer->BindShader( "Data/Shaders/Fresnel.hlsl" );
	//g_renderer->BindNormalTexture( nullptr );
	//g_renderer->SetBlendMode( eBlendMode::ALPHA );
	//g_renderer->SetDepthTest( eCompareFunc::COMPARISON_EQUAL, false );

	//g_renderer->SetMaterialData( (void*)&m_fresnelData, sizeof(m_fresnelData) );

	//model = m_sphereMeshFresnelTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_sphereMesh );
 //  
	//// Dissolve
	//g_renderer->BindTexture( USER_TEXTURE_SLOT_START, g_renderer->CreateOrGetTextureFromFile( "Data/Images/noise.png" ) );
	//g_renderer->BindShader( "Data/Shaders/Dissolve.hlsl" );
	//g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	//DissolveConstants dissolveData;
	//dissolveData.dissolveFactor = m_dissolveFactor;
	//dissolveData.edgeWidth = m_dissolveEdge;
	//dissolveData.startColor = Rgba8::RED.GetAsRGBVector();
	//dissolveData.endColor = Rgba8::BLUE.GetAsRGBVector();
	//g_renderer->SetMaterialData( ( void* )&dissolveData, sizeof( dissolveData ) );

	//model = m_cubeMeshTransformDissolve.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_cubeMesh );
	//
	//// Triplanar
	//g_renderer->BindShader( "Data/Shaders/Triplanar.hlsl" );
	//g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	//g_renderer->BindTexture( USER_TEXTURE_SLOT_START, g_renderer->CreateOrGetTextureFromFile( "Data/Images/grass_d.png" ) );
	//g_renderer->BindTexture( USER_TEXTURE_SLOT_START + 1, g_renderer->CreateOrGetTextureFromFile( "Data/Images/sand_d.png" ) );
	//g_renderer->BindTexture( USER_TEXTURE_SLOT_START + 2, g_renderer->CreateOrGetTextureFromFile( "Data/Images/wall_d.png" ) );
	//g_renderer->BindTexture( USER_TEXTURE_SLOT_START + 3, g_renderer->CreateOrGetTextureFromFile( "Data/Images/grass_n.png" ) );
	//g_renderer->BindTexture( USER_TEXTURE_SLOT_START + 4, g_renderer->CreateOrGetTextureFromFile( "Data/Images/sand_n.png" ) );
	//g_renderer->BindTexture( USER_TEXTURE_SLOT_START + 5, g_renderer->CreateOrGetTextureFromFile( "Data/Images/wall_n.png" ) );

	//model = m_sphereMeshTriplanarTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_sphereMesh );

	//// Projection
	//g_renderer->BindShader( "Data/Shaders/Projection.hlsl" );
	//g_renderer->SetBlendMode( eBlendMode::ADDITIVE );
	//g_renderer->SetDepthTest( eCompareFunc::COMPARISON_EQUAL, false );

	//ProjectionConstants projMaterial;
	//projMaterial.position = m_lights[0].light.position;

	//Mat44 view = m_projectionViewMatrix;
	//Mat44 projection = MakePerspectiveProjectionMatrixD3D( 90.f, 1.f, -.1f, -100.f );

	//projection.PushTransform( view );

	//projMaterial.projectionMatrix = projection;
	//projMaterial.power = 1.f;

	//g_renderer->SetMaterialData( (void*)&projMaterial, sizeof( projMaterial ) );
	//g_renderer->BindTexture( USER_TEXTURE_SLOT_START, g_renderer->CreateOrGetTextureFromFile( "Data/Images/test.png" ) );

	//// Redraw scene with projection
	//model = m_cubeMeshTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_cubeMesh );

	//model = m_quadMeshTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_quadMesh );

	//model = m_sphereMeshTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_sphereMesh );

	//model = m_cubeMeshTransformDissolve.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_cubeMesh );

	//model = m_sphereMeshFresnelTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_sphereMesh );

	//model = m_sphereMeshTriplanarTransform.GetAsMatrix();
	//g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	//g_renderer->DrawMesh( m_sphereMesh );

	g_renderer->EndCamera( *m_worldCamera );

	DebugRenderWorldToCamera( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeShader( int nextShaderIdx )
{
	if ( nextShaderIdx > (int)m_shaderPaths.size() - 1 )
	{
		nextShaderIdx = 0;
	}
	else if ( nextShaderIdx < 0 )
	{
		nextShaderIdx = (int)m_shaderPaths.size() - 1;
	}

	m_currentShaderIdx = nextShaderIdx;
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
bool Game::SetAmbientLightColor( EventArgs* args )
{
	s_ambientLightColor = args->GetValue( "color", Vec3( 1.f, 1.f, 1.f ) );

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Game::SetPointLightColor( EventArgs* args )
{
	s_currentLightColor = args->GetValue( "color", Vec3( 1.f, 1.f, 1.f ) );

	return false;
}

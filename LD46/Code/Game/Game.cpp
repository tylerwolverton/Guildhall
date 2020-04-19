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
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
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
#include "Game/GameObject.hpp"
#include "Game/InteractableSwitch.hpp"
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"


static float s_mouseSensitivityMultiplier = 1.f;
static Vec3 s_ambientLightColor = Vec3( 1.f, 1.f, 1.f );

static std::vector<InteractableSwitch*> s_lightSwitches;
static float s_powerLevel = .0f;
static int s_curSwitchIdx = 0;
static bool s_isPartyModeEnabled = false;


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
	g_eventSystem->RegisterEvent( "light_set_ambient_color", "Usage: light_set_ambient_color color=r,g,b", eUsageLocation::DEV_CONSOLE, SetAmbientLightColor );

	g_eventSystem->RegisterEvent( "light_switch_activated", "A light switch was flipped", eUsageLocation::EVERYWHERE, SetPowerLevel );

	g_inputSystem->PushMouseOptions( CURSOR_RELATIVE, false, true );
		
	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	EnableDebugRendering();

	m_physics2D = new Physics2D();
	m_physics2D->Startup( m_gameClock );
	m_physics2D->SetSceneGravity( 0.f );

	DiscCollider2D* discCollider = m_physics2D->CreateDiscCollider( Vec2::ZERO, m_playerRadius );
	discCollider->m_material.m_bounciness = 0.f;

	m_playerRigidbody = m_physics2D->CreateRigidbody();
	m_playerRigidbody->TakeCollider( discCollider );
	m_playerRigidbody->ChangeMass( -9.f );
	m_playerRigidbody->ChangeDrag( .08f );
	m_playerRigidbody->SetPosition( Vec2( -5.f, -5.f ) );

	m_player = new GameObject();
	m_player->SetRigidbody( m_playerRigidbody );
	m_player->EnableTransformUpdate();

	m_gameObjects.push_back( m_player );

	InitializeCameras();
	InitializeMaterials();
	InitializeMeshes();
	InitializeLights();
	InitializeObstacles();

	BuildEnvironment();
	SpawnLightSwitches();

	//m_world = new World();
	//m_world->BuildNewMap( g_gameConfigBlackboard.GetValue( "startMap", "MutateDemo" ) );

	s_lightSwitches[0]->Enable();

	m_fresnelData.color = Rgba8::GREEN.GetAsRGBVector();
	m_fresnelData.power = 32.f;

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::BeginFrame()
{
	m_physics2D->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeCameras()
{
	m_worldCamera = new Camera();
	m_worldCamera->SetColorTarget( nullptr );
	Texture* depthTexture = g_renderer->GetOrCreateDepthStencil( g_renderer->GetDefaultBackBufferSize() );
	m_worldCamera->SetDepthStencilTarget( depthTexture );

	m_worldCamera->SetOutputSize( Vec2( 16.f, 9.f ) );
	m_worldCamera->SetProjectionPerspective( 60.f, -.1f, -100.f );
	//m_worldCamera->SetProjectionOrthographic( 60.f, -.1f, -100.f );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeMeshes()
{
	// Cube
	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	AppendVertsAndIndicesForCubeMesh( vertices, indices, Vec3::ZERO, 1.f, Rgba8::WHITE );

	m_cubeMesh = new GPUMesh( g_renderer, vertices, indices );

	// Quad
	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForQuad( vertices, indices, AABB2( -1.f, -1.f, 1.f, 1.f ), Rgba8::WHITE );

	m_quadMesh = new GPUMesh( g_renderer, vertices, indices );

	// Spheres 
	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForSphereMesh( vertices, indices, Vec3::ZERO, 1.f, 64, 64, Rgba8::WHITE );

	m_sphereMesh = new GPUMesh( g_renderer, vertices, indices );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeMaterials()
{
	// Wall
	m_wallMaterial = new Material();
	m_wallMaterial->SetShader( g_renderer->GetOrCreateShader( "Data/Shaders/Lit.hlsl" ) );
	m_wallMaterial->SetDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/factory_wall_d.png" ) );
	m_wallMaterial->SetNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/factory_wall_n.png" ) );

	// Floor
	m_floorMaterial = new Material();
	m_floorMaterial->SetShader( g_renderer->GetOrCreateShader( "Data/Shaders/Lit.hlsl" ) );
	m_floorMaterial->SetDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/floor_tiles_d.png" ) );
	m_floorMaterial->SetNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/floor_tiles_n.png" ) );

	// Ceiling
	m_ceilingMaterial = new Material();
	m_ceilingMaterial->SetShader( g_renderer->GetOrCreateShader( "Data/Shaders/Lit.hlsl" ) );
	m_ceilingMaterial->SetDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/yellow_plaster_d.png" ) );
	m_ceilingMaterial->SetNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/yellow_plaster_n.png" ) );

	// No texture
	m_whiteMaterial = new Material();
	m_whiteMaterial->SetShader( g_renderer->GetOrCreateShader( "Data/Shaders/Lit.hlsl" ) );
	m_whiteMaterial->SetDiffuseTexture( nullptr );
	m_whiteMaterial->SetNormalTexture( nullptr );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeLights()
{
	// Light 0 must be active switch light
	m_activeSwitchLight.color = Rgba8::GREEN.GetAsRGBVector();
	m_activeSwitchLight.intensity = .5f;

	Vec3 attentuation = Vec3( 1.f, 0.f, 0.f );

	Light overheadLight0;
	overheadLight0.position = Vec3( 5.f, 3.75f, 5.f );
	//overheadLight0.color = Rgba8::GREEN.GetAsRGBVector();
	overheadLight0.intensity = 1.f;
	overheadLight0.attenuation = attentuation;

	Light overheadLight1;
	overheadLight1.position = Vec3( 5.f, 3.75f, -5.f );
	//overheadLight1.color = Rgba8::BLUE.GetAsRGBVector();
	overheadLight1.intensity = 1.f;
	overheadLight1.attenuation = attentuation;
	
	Light overheadLight2;
	overheadLight2.position = Vec3( -5.f, 3.75f, 5.f );
	//overheadLight2.color = Rgba8::RED.GetAsRGBVector();
	overheadLight2.intensity = 1.f;
	overheadLight2.attenuation = attentuation;

	Light overheadLight3;
	overheadLight3.position = Vec3( -5.f, 3.75f, -5.f );
	//overheadLight3.color = Rgba8::YELLOW.GetAsRGBVector();
	overheadLight3.intensity = 1.f;
	overheadLight3.attenuation = attentuation;

	Light overheadLight4;
	overheadLight4.position = Vec3( -2.5f, 3.75f, 2.5f );
	overheadLight4.color = Rgba8::PURPLE.GetAsRGBVector();
	overheadLight4.intensity = 1.f;
	overheadLight4.attenuation = attentuation;

	Light overheadLight5;
	overheadLight5.position = Vec3( 2.5f, 3.75f, -2.5f );
	overheadLight5.color = Rgba8::ORANGE.GetAsRGBVector();
	overheadLight5.intensity = 1.f;
	overheadLight5.attenuation = attentuation;

	Light overheadLight6;
	overheadLight6.position = Vec3( 0.f, 3.75f, 0.f );
	overheadLight6.color = Rgba8::CYAN.GetAsRGBVector();
	overheadLight6.intensity = 1.f;
	overheadLight6.attenuation = attentuation;

	m_lights[1] = overheadLight0;
	m_lights[2] = overheadLight1;
	m_lights[3] = overheadLight2;
	m_lights[4] = overheadLight3;
	//m_lights[5] = overheadLight4;
	//m_lights[6] = overheadLight5;
	//m_lights[7] = overheadLight6;
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeObstacles()
{
	//SpawnEnvironmentBall( Vec3( 0.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 0.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
	//SpawnEnvironmentBall( Vec3( 3.f, 0.f, -5.f ), .5f );
}


//-----------------------------------------------------------------------------------------------
void Game::BuildEnvironment()
{
	m_floorTransform.SetPosition( Vec3( 0.f, -.5f, 0.f ) );
	m_floorTransform.SetScale( Vec3( 20.f, .1f, 20.f ) );

	GameObject* floor = new GameObject();
	floor->SetMaterial( m_floorMaterial );
	floor->SetMesh( m_cubeMesh );
	floor->SetTransform( m_floorTransform );

	m_gameObjects.push_back( floor );

	Transform ceilingTransform = m_floorTransform;
	ceilingTransform.Translate( Vec3( 0.f, 4.f, 0.f ) );

	GameObject* ceiling = new GameObject();
	ceiling->SetMaterial( m_ceilingMaterial );
	ceiling->SetMesh( m_cubeMesh );
	ceiling->SetTransform( ceilingTransform );

	m_gameObjects.push_back( ceiling );


	/*SpawnEnvironmentBox( Vec3( ( 10.f + WALL_THICKNESS * .5f ), 0.f, 0.f ), Vec3( WALL_THICKNESS, 8.f, 20.f + WALL_THICKNESS * 2.f ) );
	SpawnEnvironmentBox( Vec3( -( 10.f + WALL_THICKNESS * .5f ), 0.f, 0.f ), Vec3( WALL_THICKNESS, 8.f, 20.f + WALL_THICKNESS * 2.f ) );
	SpawnEnvironmentBox( Vec3( 0.f, 0.f, -( 10.f + WALL_THICKNESS * .5f ) ), Vec3( 20.f + WALL_THICKNESS * 2.f, 8.f, WALL_THICKNESS ) );
	SpawnEnvironmentBox( Vec3( 0.f, 0.f, ( 10.f + WALL_THICKNESS * .5f ) ), Vec3( 20.f + WALL_THICKNESS * 2.f, 8.f, WALL_THICKNESS ) );*/
	
	// Outer wall
	SpawnEnvironmentBox( Vec3( ( MAP_WIDTH_HALF + WALL_THICKNESS * .5f ),	0.f, 0.f ),											Vec3( WALL_THICKNESS,						8.f, MAP_HEIGHT + WALL_THICKNESS * 2.f ) );
	SpawnEnvironmentBox( Vec3( -( MAP_WIDTH_HALF + WALL_THICKNESS * .5f ),	0.f, 0.f ),											Vec3( WALL_THICKNESS,						8.f, MAP_HEIGHT + WALL_THICKNESS * 2.f ) );
	SpawnEnvironmentBox( Vec3( 0.f,											0.f, -( MAP_HEIGHT_HALF + WALL_THICKNESS * .5f ) ),	Vec3( MAP_HEIGHT + WALL_THICKNESS * 2.f,	8.f, WALL_THICKNESS ) );
	SpawnEnvironmentBox( Vec3( 0.f,											0.f, ( MAP_HEIGHT_HALF + WALL_THICKNESS * .5f ) ),	Vec3( MAP_HEIGHT + WALL_THICKNESS * 2.f,	8.f, WALL_THICKNESS ) );

	// Central wall
	SpawnEnvironmentBox( Vec3::ZERO, Vec3( MAP_WIDTH * .8f, 
										   8.f, 
										   WALL_THICKNESS ) );

	// Top half
	Vec3 topHalfStart( -3.f, -.3f, -5.f );
	float wallHeight = .6f;

	SpawnEnvironmentBox( topHalfStart + Vec3( 0.f, 0.f, -3.f ), Vec3( WALL_THICKNESS,
																	  wallHeight,
																	  4.f ) );
	
	SpawnEnvironmentBox( topHalfStart + Vec3( 0.f, 0.f, 3.f ), Vec3( WALL_THICKNESS,
																	 wallHeight,
																	  4.f ) );

	SpawnEnvironmentBox( topHalfStart + Vec3( 2.f, 0.f, 0.f ), Vec3( WALL_THICKNESS,
																	 wallHeight,
																	 6.5f ) );

	SpawnEnvironmentBox( topHalfStart + Vec3( 4.f, 0.f, 0.f ), Vec3( 3.f,
																	 wallHeight,
																	 WALL_THICKNESS ) );

	SpawnEnvironmentBox( topHalfStart + Vec3( 5.f, 0.f, 3.f ), Vec3( WALL_THICKNESS,
																	 wallHeight,
																	 5.f ) );

	SpawnEnvironmentBox( topHalfStart + Vec3( 6.5f, 0.f, -3.f ), Vec3( 2.f,
																	   wallHeight,
																	   2.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnLightSwitches()
{
	Vec3 switchScale( .1f, .1f, .01f );
	float switchHeight = -.15;

	//SpawnSwitch( Vec3( -5.f,				0.f, -MAP_HEIGHT_HALF ),	Vec3( 0.f, 0.f, 180.f ),	switchScale );
	//SpawnSwitch( Vec3( MAP_WIDTH_HALF,	0.f, 0.f ),					Vec3( 0.f, 0.f, 90.f ),		switchScale );
	//SpawnSwitch( Vec3( 0.f,				0.f, MAP_HEIGHT_HALF ),		Vec3( 0.f, 0.f, 0.f ),		switchScale );
	//SpawnSwitch( Vec3( -MAP_WIDTH_HALF, 0.f, 0.f ),					Vec3( 0.f, 0.f, 270.f ),	switchScale );

	SpawnSwitch( Vec3( -5.f, switchHeight, -MAP_HEIGHT_HALF ), Vec3( 0.f, 0.f, 180.f ), switchScale );
	SpawnSwitch( Vec3( 1.5f, switchHeight, -3.f ), Vec3( 0.f, 0.f, 90.f ), switchScale );


	SpawnSwitch( Vec3( MAP_WIDTH_HALF, switchHeight, 0.f ), Vec3( 0.f, 0.f, 90.f ), switchScale );
	SpawnSwitch( Vec3( 0.f, switchHeight, MAP_HEIGHT_HALF ), Vec3( 0.f, 0.f, 0.f ), switchScale );
	SpawnSwitch( Vec3( -MAP_WIDTH_HALF, switchHeight, 0.f ), Vec3( 0.f, 0.f, 270.f ), switchScale );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	m_physics2D->Shutdown();

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
	PTR_VECTOR_SAFE_DELETE( m_gameObjects );
	PTR_SAFE_DELETE( m_quadMesh );
	PTR_SAFE_DELETE( m_cubeMesh );
	PTR_SAFE_DELETE( m_sphereMesh );
	PTR_SAFE_DELETE( m_wallMaterial );
	PTR_SAFE_DELETE( m_floorMaterial );
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
	UpdateLights();
	//float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();
	
	for ( int gameObjIdx = 0; gameObjIdx < (int)m_gameObjects.size(); ++gameObjIdx )
	{
		if ( m_gameObjects[gameObjIdx] != nullptr )
		{
			m_gameObjects[gameObjIdx]->Update();
		}
	}

	m_physics2D->Update();

	for ( int gameObjIdx = 0; gameObjIdx < (int)m_gameObjects.size(); ++gameObjIdx )
	{
		if ( m_gameObjects[gameObjIdx] != nullptr )
		{
			m_gameObjects[gameObjIdx]->UpdateTransform();
		}
	}

	m_worldCamera->SetPosition( m_player->GetPosition() );

	//if ( m_player->GetPosition().z > MAP_HEIGHT_HALF )
	//{
	//	m_playerRigidbody->SetPosition( Vec2( m_playerRigidbody->GetPosition().x, MAP_HEIGHT_HALF ) );
	//}
	//if ( m_player->GetPosition().x < -MAP_WIDTH_HALF )
	//{
	//	m_playerRigidbody->SetPosition( Vec2( -MAP_WIDTH_HALF, m_playerRigidbody->GetPosition().y ) );
	//}
	//if ( m_player->GetPosition().x > MAP_WIDTH_HALF )
	//{
	//	m_playerRigidbody->SetPosition( Vec2( MAP_WIDTH_HALF, m_playerRigidbody->GetPosition().y ) );
	//}

	//m_worldCamera->SetPosition( Vec3( m_playerRigidbody->GetPosition().x, 0.f, m_playerRigidbody->GetPosition().y ) );

	//WorldWireSphere( Vec3( m_playerRigidbody->GetPosition().x, 0.f, m_playerRigidbody->GetPosition().y ), m_playerRadius, Rgba8::GREEN );

	//m_world->Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

	UpdateCameraTransform( deltaSeconds );
	UpdateDebugDrawCommands();

	if ( g_inputSystem->WasKeyJustPressed( KEY_F4 ) )
	{
		g_renderer->ReloadShaders();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameraTransform( float deltaSeconds )
{
	Vec3 cameraTranslation;

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

	if ( g_inputSystem->IsKeyPressed( KEY_SHIFT ) )
	{
		//cameraTranslation *= 2.f;
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

	// Translation
	TranslateCameraFPS( cameraTranslation * deltaSeconds );


	if ( g_inputSystem->IsKeyPressed( 'E' ) )
	{
		InteractableSwitch* lightSwitch = m_player->GetSelectedSwitch();
		if ( lightSwitch != nullptr 
			 && lightSwitch->IsEnabled() )
		{
			lightSwitch->Activate();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateDebugDrawCommands()
{
	
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateLights()
{
	float deltaSeconds = m_gameClock->GetLastDeltaSeconds();
	s_powerLevel -= .025f * deltaSeconds;

	for ( int lightIdx = 1; lightIdx < NUM_GAME_LIGHTS; ++lightIdx )
	{
		if ( m_lights[lightIdx].intensity > 0.01f )
		{
			m_lights[lightIdx].intensity = s_powerLevel;
		}
	}

	if ( s_isPartyModeEnabled )
	{
		for ( int lightIdx = 1; lightIdx < NUM_GAME_LIGHTS; ++lightIdx )
		{
			m_lights[lightIdx].intensity = 1.f;
		}

		m_activeSwitchLight.intensity = 0.f;
	}
	else
	{
		m_activeSwitchLight.position = s_lightSwitches[s_curSwitchIdx]->GetPosition() + s_lightSwitches[s_curSwitchIdx]->GetForwardVector() * .1f;
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

	// Lock player to moving on the ground only
	absoluteTranslation.y = 0.f;

	//m_worldCamera->Translate( absoluteTranslation );

	Vec2 cameraPosition = m_worldCamera->GetTransform().GetPosition().XZ();
	//m_playerRigidbody->SetPosition( cameraPosition );
	//m_player->SetTransform( m_worldCamera->GetTransform() );
	
	m_player->Translate( absoluteTranslation * 3.5f );
	//m_player->ApplyImpulseAt( absoluteTranslation * 50.f );
	m_player->SetOrientation( m_worldCamera->GetTransform().m_orientation );
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnEnvironmentBox( const Vec3& location, const Vec3& dimensions, eSimulationMode simMode )
{
	Transform wallTransform;
	wallTransform.SetPosition( location );
	wallTransform.SetScale( dimensions );

	Vec2 polygonPoints[4];
	float minX = wallTransform.GetPosition().x - dimensions.x * .5f;
	float minY = -wallTransform.GetPosition().z - dimensions.z * .5f;
	float maxX = wallTransform.GetPosition().x + dimensions.x * .5f;
	float maxY = -wallTransform.GetPosition().z + dimensions.z * .5f;

	Polygon2 polygon;
	AABB2 boundingBox( minX, minY, maxX, maxY );
	boundingBox.GetCornerPositionsCCW( polygonPoints );
	polygon.SetPoints( polygonPoints, 4 );
	PolygonCollider2D* polygonCollider = m_physics2D->CreatePolygon2Collider( polygon );
	polygonCollider->m_material.m_bounciness = 1.f;
	polygonCollider->m_material.m_friction = 0.f;

	Rigidbody2D* wallRigidbody = m_physics2D->CreateRigidbody();
	wallRigidbody->SetSimulationMode( simMode );
	wallRigidbody->SetPosition( wallTransform.GetPosition().XZ() );

	wallRigidbody->TakeCollider( polygonCollider );

	m_wallTransforms.push_back( wallTransform );

	GameObject* gameObject = new GameObject();
	gameObject->SetRigidbody( wallRigidbody );
	gameObject->SetMaterial( m_wallMaterial );
	gameObject->SetMesh( m_cubeMesh );
	gameObject->SetTransform( wallTransform );

	m_gameObjects.push_back( gameObject );
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnEnvironmentBall( const Vec3& location, float radius, eSimulationMode simMode )
{
	Transform ballTransform;
	ballTransform.SetPosition( location );
	ballTransform.SetScale( Vec3( radius, radius, radius ) );

	DiscCollider2D* discCollider = m_physics2D->CreateDiscCollider( Vec2( 0.f, 0.f ), radius );
	discCollider->m_material.m_friction = 0.f;
	discCollider->m_material.m_bounciness = 1.f;

	Rigidbody2D* ballRigidbody = m_physics2D->CreateRigidbody();
	ballRigidbody->SetSimulationMode( simMode );
	ballRigidbody->SetPosition( ballTransform.GetPosition().XZ() );
	ballRigidbody->ChangeMass( -5.f );

	ballRigidbody->TakeCollider( discCollider );
	ballRigidbody->SetVelocity( Vec2( 10.f, 1.f ) );

	GameObject* gameObject = new GameObject();
	gameObject->SetRigidbody( ballRigidbody );
	gameObject->SetMaterial( m_whiteMaterial );
	gameObject->SetMesh( m_sphereMesh );
	gameObject->SetTransform( ballTransform );
	gameObject->EnableTransformUpdate();

	m_gameObjects.push_back( gameObject );
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnSwitch( const Vec3& location, const Vec3& orientation, const Vec3& dimensions )
{
	Transform switchTransform;
	switchTransform.SetPosition( location );
	switchTransform.SetOrientation( orientation );
	switchTransform.SetScale( dimensions );
	
	InteractableSwitch* gameSwitch = new InteractableSwitch( m_player, .25f );
	gameSwitch->SetMaterial( m_whiteMaterial );
	gameSwitch->SetMesh( m_cubeMesh );
	gameSwitch->SetTransform( switchTransform );

	s_lightSwitches.push_back( gameSwitch );
	m_gameObjects.push_back( gameSwitch );
}


//-----------------------------------------------------------------------------------------------
void Game::EnableNextSwitch()
{
	s_curSwitchIdx++;

	if ( s_curSwitchIdx == (int)s_lightSwitches.size() )
	{
		s_curSwitchIdx = 0;
		s_isPartyModeEnabled = true;
		return;
	}

	s_lightSwitches[s_curSwitchIdx]->Enable();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	g_renderer->SetSampler( eSampler::POINT_WRAP );
	g_renderer->BindSampler( nullptr );
	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );
	
	g_renderer->DisableAllLights();
	g_renderer->SetAmbientLight( s_ambientLightColor, m_ambientIntensity );
	g_renderer->SetGamma( m_gamma );
	
	for ( int lightIdx = 0; lightIdx < NUM_GAME_LIGHTS; ++lightIdx )
	{
		//if ( m_lights[lightIdx].isEnabled )
		//{
			g_renderer->EnableLight( lightIdx, m_lights[lightIdx] );
		//}
	}
	
	
	for ( int gameObjIdx = 0; gameObjIdx < (int)m_gameObjects.size(); ++gameObjIdx )
	{
		if ( m_gameObjects[gameObjIdx] != nullptr )
		{
			m_gameObjects[gameObjIdx]->RenderWithMaterial();
		}
	}

	//m_world->Render();

	g_renderer->EndCamera( *m_worldCamera );

	DebugRenderWorldToCamera( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::EndFrame()
{
	m_physics2D->EndFrame();
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets...", Rgba8::WHITE );

	g_renderer->GetOrCreateShader( "Data/Shaders/Lit.hlsl" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/brick_normal.png" );

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
bool Game::SetPowerLevel( EventArgs* args )
{
	//s_powerLevel = .25f;
	EnableNextSwitch();

	return false;
}

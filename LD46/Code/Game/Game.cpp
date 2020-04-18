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
#include "Game/World.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"


static float s_mouseSensitivityMultiplier = 1.f;
static Vec3 s_ambientLightColor = Vec3( 1.f, 1.f, 1.f );


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

	g_inputSystem->PushMouseOptions( CURSOR_RELATIVE, false, true );
		
	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	EnableDebugRendering();

	m_physics2D = new Physics2D();
	m_physics2D->Startup( m_gameClock );
	m_physics2D->SetSceneGravity( 0.f );

	m_playerRigidbody = m_physics2D->CreateRigidbody();
	DiscCollider2D* discCollider = m_physics2D->CreateDiscCollider( Vec2::ZERO, m_playerRadius );
	m_playerRigidbody->TakeCollider( discCollider );

	InitializeCameras();
	InitializeMaterials();
	InitializeMeshes();

	//m_world = new World();
	//m_world->BuildNewMap( g_gameConfigBlackboard.GetValue( "startMap", "MutateDemo" ) );

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

	m_cubeMeshTransform.SetPosition( Vec3( 0.f, 0.f, -6.f ) );

	m_cubeRigidbody = m_physics2D->CreateRigidbody();
	m_cubeRigidbody->SetSimulationMode( SIMULATION_MODE_STATIC );
	m_cubeRigidbody->SetPosition( m_cubeMeshTransform.GetPosition().XZ() );

	Vec2 polygonPoints[4];
	float minX = m_cubeMeshTransform.GetPosition().x - .5f;
	float minY = -m_cubeMeshTransform.GetPosition().z - .5f;
	float maxX = m_cubeMeshTransform.GetPosition().x + .5f;
	float maxY = -m_cubeMeshTransform.GetPosition().z + .5f;

	Polygon2 polygon;
	AABB2 boundingBox( minX, minY, maxX, maxY );
	boundingBox.GetCornerPositionsCCW( polygonPoints );
	polygon.SetPoints( polygonPoints, 4 );
	PolygonCollider2D* polygonCollider = m_physics2D->CreatePolygon2Collider( polygon );
	
	m_cubeRigidbody->TakeCollider( polygonCollider );

	SpawnEnvironmentBox( Vec3( 10.f, 0.f, 0.f ), Vec3( 1.f, 1.f, 20.f ) );
	SpawnEnvironmentBox( Vec3( -10.f, 0.f, 0.f ), Vec3( 1.f, 1.f, 20.f ) );
	SpawnEnvironmentBox( Vec3( 0.f, 0.f, -10.f ), Vec3( 20.f, 1.f, 1.f ) );
	SpawnEnvironmentBox( Vec3( 0.f, 0.f, 10.f ), Vec3( 20.f, 1.f, 1.f ) );

	m_floorTransform.SetPosition( Vec3( 0.f, -.5f, 0.f ) );
	m_floorTransform.SetScale( Vec3( 20.f, .1f, 20.f ) );

	GameObject floor;
	floor.SetMaterial( m_floorMaterial );
	floor.SetMesh( m_cubeMesh );
	floor.SetTransform( m_floorTransform );

	m_gameObjects.push_back( floor );

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
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	m_physics2D->Shutdown();

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
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

	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();
	
	m_physics2D->Update();

	m_worldCamera->SetPosition( Vec3( m_playerRigidbody->GetPosition().x, 0.f, m_playerRigidbody->GetPosition().y ) );

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
		cameraTranslation *= 10.f;
	}

	// Rotation
	Vec2 mousePosition = g_inputSystem->GetMouseDeltaPosition();
	float yaw = -mousePosition.x * s_mouseSensitivityMultiplier;
	float pitch = -mousePosition.y * s_mouseSensitivityMultiplier;
	yaw *= .009f;
	pitch *= .009f;

	Transform transform = m_worldCamera->GetTransform();
	m_worldCamera->SetPitchRollYawRotation( transform.m_rotation.x + pitch,
											0.f,
											transform.m_rotation.z + yaw );

	// Translation
	TranslateCameraFPS( cameraTranslation * deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateDebugDrawCommands()
{
	
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

	m_worldCamera->Translate( absoluteTranslation );

	Vec2 cameraPosition = m_worldCamera->GetTransform().GetPosition().XZ();
	m_playerRigidbody->SetPosition( cameraPosition );
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

	Rigidbody2D* wallRigidbody = m_physics2D->CreateRigidbody();
	wallRigidbody->SetSimulationMode( simMode );
	wallRigidbody->SetPosition( wallTransform.GetPosition().XZ() );

	wallRigidbody->TakeCollider( polygonCollider );

	m_wallTransforms.push_back( wallTransform );

	GameObject gameObject;
	gameObject.SetRigidbody( wallRigidbody );
	gameObject.SetMaterial( m_wallMaterial );
	gameObject.SetMesh( m_cubeMesh );
	gameObject.SetTransform( wallTransform );

	m_gameObjects.push_back( gameObject );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	/*g_renderer->BindDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/factory_wall_d.png" ) );
	g_renderer->BindNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/factory_wall_n.png" ) );*/
	g_renderer->SetSampler( eSampler::POINT_WRAP );
	g_renderer->BindSampler( nullptr );
	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	//g_renderer->BindShader( g_renderer->GetOrCreateShader( "Data/Shaders/Lit.hlsl" ) );

	
	g_renderer->DisableAllLights();
	g_renderer->SetAmbientLight( s_ambientLightColor, m_ambientIntensity );
	g_renderer->SetGamma( m_gamma );
	
	// Render normal objects
	Mat44 model = m_cubeMeshTransform.GetAsMatrix();
	g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_renderer->DrawMesh( m_cubeMesh );
	
	for ( int gameObjIdx = 0; gameObjIdx < (int)m_gameObjects.size(); ++gameObjIdx )
	{
		m_gameObjects[gameObjIdx].Render();
	}

	/*for ( int transformIdx = 0; transformIdx < (int)m_wallTransforms.size(); ++transformIdx )
	{
		model = m_wallTransforms[transformIdx].GetAsMatrix();
		g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
		g_renderer->DrawMesh( m_cubeMesh );
	}*/

	// Draw floor
	/*g_renderer->BindDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/floor_tiles_d.png" ) );
	g_renderer->BindNormalTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/Textures/floor_tiles_n.png" ) );

	model = m_floorTransform.GetAsMatrix();
	g_renderer->SetModelData( model, Rgba8::WHITE, m_specularFactor, m_specularPower );
	g_renderer->DrawMesh( m_cubeMesh );*/
	

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

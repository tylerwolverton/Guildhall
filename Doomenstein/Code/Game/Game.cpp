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
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
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


//-----------------------------------------------------------------------------------------------
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
	Transform::s_axisOrientation.m_axisYawPitchRollOrder = eAxisYawPitchRollOrder::ZYX;

	//Transform::s_identityOrientation = MakeLookAtMatrix(Vec3::ZERO, Vec3(1.f, 0.f, 0.f), Vec3(0.f, 0.f, 1.f));
	Transform::s_identityOrientation.PushTransform( Mat44::CreateZRotationDegrees( -90.f ) );
	Transform::s_identityOrientation.PushTransform( Mat44::CreateXRotationDegrees( 90.f ) );

	g_eventSystem->RegisterEvent( "set_mouse_sensitivity", "Usage: set_mouse_sensitivity multiplier=NUMBER. Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );
	g_eventSystem->RegisterEvent( "light_set_ambient_color", "Usage: light_set_ambient_color color=r,g,b", eUsageLocation::DEV_CONSOLE, SetAmbientLightColor );

	g_inputSystem->PushMouseOptions( CURSOR_RELATIVE, false, true );
		
	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	EnableDebugRendering();
	m_isDebugRendering = true;

	InitializeCameras();
	InitializeMeshes();

	LoadAssets();

	m_world = new World( m_gameClock );

	m_curMapStr = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_curMapStr );
	g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_curMapStr.c_str() ) );
	m_world->BuildNewMap( m_curMapStr );

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeCameras()
{
	m_worldCamera = new Camera();
	Texture* depthTexture = g_renderer->GetOrCreateDepthStencil( g_renderer->GetDefaultBackBufferSize() );
	m_worldCamera->SetDepthStencilTarget( depthTexture );

	m_worldCamera->SetOutputSize( Vec2( 16.f, 9.f ) );
	m_worldCamera->SetProjectionPerspective( 60.f, -.05f, -100.f );
	m_worldCamera->Translate( Vec3( 0.f, 0.f, .5f ) );

	Rgba8 backgroundColor( 10, 10, 10, 255 );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, backgroundColor );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeMeshes()
{
	// Cube
	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	AppendVertsAndIndicesForCubeMesh( vertices, indices, Vec3::ZERO, 1.f, Rgba8::WHITE );

	m_cubeMesh = new GPUMesh( g_renderer, vertices, indices );

	Transform cubeTransform;
	cubeTransform.SetPosition( Vec3( 2.5f, 0.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );

	cubeTransform.SetPosition( Vec3( 2.5f, 2.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );

	cubeTransform.SetPosition( Vec3( 0.5f, 2.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );
	
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

	// Initialize materials
	m_testMaterial = new Material( g_renderer, "Data/Materials/Test.material" );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
	PTR_SAFE_DELETE( m_testMaterial );
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

	m_world->Update();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

	UpdateCameraTransform( deltaSeconds );
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
	{
		/*float volume = m_rng->RollRandomFloatInRange( .5f, 1.f );
		float balance = m_rng->RollRandomFloatInRange( -1.f, 1.f );
		float speed = m_rng->RollRandomFloatInRange( .5f, 2.f );
		
		g_audioSystem->PlaySound( m_testSound, false, volume, balance, speed );*/

		m_isDebugRendering = !m_isDebugRendering;
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

	if ( g_inputSystem->IsKeyPressed( 'D' ) )
	{
		cameraTranslation.y += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'A' ) )
	{
		cameraTranslation.y -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'W' ) )
	{
		cameraTranslation.x += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		cameraTranslation.x -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'E' ) )
	{
		cameraTranslation.z += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'Q' ) )
	{
		cameraTranslation.z -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SHIFT ) )
	{
		cameraTranslation *= 10.f;
	}

	// Rotation
	Vec2 mousePosition = g_inputSystem->GetMouseDeltaPosition();
	float yawDegrees = -mousePosition.x * s_mouseSensitivityMultiplier;
	float pitchDegrees = mousePosition.y * s_mouseSensitivityMultiplier;
	yawDegrees *= .009f;
	pitchDegrees *= .009f;

	Transform transform = m_worldCamera->GetTransform();
	m_worldCamera->RotateYawPitchRoll( yawDegrees, pitchDegrees, 0.f );

	// Translation
	TranslateCameraFPS( cameraTranslation * deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{

}


//-----------------------------------------------------------------------------------------------
void Game::TranslateCameraFPS( const Vec3& relativeTranslation )
{
	Vec2 forwardVec = Vec2::MakeFromPolarDegrees( m_worldCamera->GetTransform().GetYawDegrees() );
	Vec2 rightVec = forwardVec.GetRotatedMinus90Degrees();

	Vec2 translationXY( relativeTranslation.x * forwardVec  
						+ relativeTranslation.y * rightVec );
	
	Vec3 absoluteTranslation( translationXY, relativeTranslation.z );

	m_worldCamera->Translate( absoluteTranslation );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	Texture* backbuffer = g_renderer->GetBackBuffer();
	Texture* colorTarget = g_renderer->AcquireRenderTargetMatching( backbuffer );

	m_worldCamera->SetColorTarget( 0, colorTarget );

	g_renderer->BeginCamera( *m_worldCamera );

	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	g_renderer->DisableAllLights();
	g_renderer->SetAmbientLight( s_ambientLightColor, m_ambientIntensity );
	g_renderer->SetGamma( m_gamma );

	m_world->Render();
	//for ( int cubeMeshTransformIdx = 0; cubeMeshTransformIdx < (int)m_cubeMeshTransforms.size(); ++cubeMeshTransformIdx )
	//{
	//	Mat44 modelMatrix = m_cubeMeshTransforms[cubeMeshTransformIdx].GetAsMatrix();
	//	g_renderer->SetModelMatrix( modelMatrix );
	//	//g_renderer->BindTexture( 0, g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
	//	g_renderer->BindMaterial( m_testMaterial );
	//	g_renderer->DrawMesh( m_cubeMesh );
	//}

	g_renderer->EndCamera( *m_worldCamera );

	// Copy rendered data to backbuffer and set on camera
	g_renderer->CopyTexture( backbuffer, colorTarget );
	m_worldCamera->SetColorTarget( backbuffer );

	g_renderer->ReleaseRenderTarget( colorTarget );

	// Debug rendering
	if ( m_isDebugRendering )
	{
		DebugRender();
	}

	DebugRenderWorldToCamera( m_worldCamera );
	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	RenderDebugUI();

	DebugAddWorldBasis( Mat44::IDENTITY, 0.f, DEBUG_RENDER_ALWAYS );

	Mat44 compassMatrix = Mat44::CreateTranslation3D( m_worldCamera->GetTransform().GetPosition() + .1f * m_worldCamera->GetTransform().GetForwardVector() );
	DebugAddWorldBasis( compassMatrix, .01f, 0.f, Rgba8::WHITE, Rgba8::WHITE, DEBUG_RENDER_ALWAYS );

	//m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderDebugUI() const
{
	// Camera position and orientation
	Transform cameraTransform = m_worldCamera->GetTransform();

	std::string cameraOrientationStr = Stringf( "Yaw: %.2f, Pitch: %.2f, Roll: %.2f",
												cameraTransform.GetYawDegrees(),
												cameraTransform.GetPitchDegrees(),
												cameraTransform.GetRollDegrees() );

	std::string cameraPositionStr = Stringf( "xyz=( %.2f, %.2f, %.2f )",
											 cameraTransform.GetPosition().x,
											 cameraTransform.GetPosition().y,
											 cameraTransform.GetPosition().z );

	DebugAddScreenTextf( Vec4( 0.f, .97f, 0.f, 0.f ), Vec2::ZERO, 20.f, Rgba8::YELLOW, 0.f,
						 "Camera - %s     %s",
						 cameraOrientationStr.c_str(),
						 cameraPositionStr.c_str() );

	// Basis text
	Mat44 cameraOrientationMatrix = cameraTransform.GetOrientationAsMatrix();
	DebugAddScreenTextf( Vec4( 0.f, .94f, 0.f, 0.f ), Vec2::ZERO, 20.f, Rgba8::RED, 0.f,
						 "iBasis ( forward +x world east when identity  )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetIBasis3D().x,
						 cameraOrientationMatrix.GetIBasis3D().y,
						 cameraOrientationMatrix.GetIBasis3D().z );

	DebugAddScreenTextf( Vec4( 0.f, .91f, 0.f, 0.f ), Vec2::ZERO, 20.f, Rgba8::GREEN, 0.f,
						 "jBasis ( left    +y world north when identity )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetJBasis3D().x,
						 cameraOrientationMatrix.GetJBasis3D().y,
						 cameraOrientationMatrix.GetJBasis3D().z );

	DebugAddScreenTextf( Vec4( 0.f, .88f, 0.f, 0.f ), Vec2::ZERO, 20.f, Rgba8::BLUE, 0.f,
						 "kBasis ( up      +z world up when identity    )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetKBasis3D().x,
						 cameraOrientationMatrix.GetKBasis3D().y,
						 cameraOrientationMatrix.GetKBasis3D().z );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets...", Rgba8::WHITE );

	// Audio
	m_testSound = g_audioSystem->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
	g_audioSystem->CreateOrGetSound( "Data/Audio/Teleporter.wav" );

	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/ViewModelsSpriteSheet_8x8.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" );
	g_renderer->CreateOrGetTextureFromFile( "Data/Images/Hud_Base.png" );

	LoadXmlMaps();

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlMaps()
{
	g_devConsole->PrintString( "Loading Maps..." );

	std::string folderPath( "Data/Maps" );

	Strings mapFiles = GetFileNamesInFolder( folderPath, "*.xml" );
	for ( int mapIdx = 0; mapIdx < (int)mapFiles.size(); ++mapIdx )
	{
		std::string& mapName = mapFiles[mapIdx];

		std::string mapFullPath( folderPath );
		mapFullPath += "/";
		mapFullPath += mapName;

		XmlDocument doc;
		XmlError loadError = doc.LoadFile( mapFullPath.c_str() );
		if ( loadError != tinyxml2::XML_SUCCESS )
		{
			ERROR_AND_DIE( Stringf( "The maps xml file '%s' could not be opened.", mapFullPath.c_str() ) );
		}

		XmlElement* root = doc.RootElement();
		
		MapDefinition* mapDef = new MapDefinition( *root, GetFileNameWithoutExtension( mapName ) );
		MapDefinition::s_definitions[mapDef->GetName()] = mapDef;

	}

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadNewMap( const std::string& mapName )
{
	//PTR_SAFE_DELETE( m_world );

	//m_world = new World( m_gameClock );
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
void Game::SetCameraPositionAndYaw( const Vec2& pos, float yaw )
{
	Transform newTransform = m_worldCamera->GetTransform();
	newTransform.SetPosition( Vec3( pos, 0.5f ) );
	newTransform.SetOrientationFromPitchRollYawDegrees( 0.f, 0.f, yaw );

	m_worldCamera->SetTransform( newTransform );
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

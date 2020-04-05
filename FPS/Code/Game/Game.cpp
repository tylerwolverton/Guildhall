#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/RenderContext.hpp"
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


static float s_mouseSensitivityMultiplier = 10.f;


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

	// Create Sphere and Cube
	std::vector<Vertex_PCUTBN> vertices;
	AppendVertsForCubeMesh( vertices, Vec3::ZERO, 2.f, Rgba8::WHITE );
	
	std::vector<uint> indices;
	AppendIndicesForCubeMesh( indices );	

	m_cubeMesh = new GPUMesh( g_renderer, vertices, indices );
	m_cubeMeshTransform.SetPosition( Vec3( -5.f, 0.f, -6.f ) );
	
	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForSphereMesh( vertices, indices, Vec3::ZERO, 1.f, 64, 32, Rgba8::WHITE );

	m_sphereMesh = new GPUMesh( g_renderer, vertices, indices );
	Transform centerTransform;
	centerTransform.SetPosition( Vec3( 5.f, 0.f, -6.f ) );
	m_sphereMeshTransform = centerTransform;
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	TileDefinition::s_definitions.clear();
	
	// Clean up member variables
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
	m_cubeMeshTransform.SetRotationFromPitchRollYawDegrees( 0.f, 0.f,  (float)( GetCurrentTimeSeconds() * 20.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	g_renderer->BindTexture( nullptr );
	g_renderer->BindShader( "Data/Shaders/Default.hlsl" );
	
	Mat44 model = m_cubeMeshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model, Rgba8::YELLOW );
	g_renderer->DrawMesh( m_cubeMesh );


	//Texture* texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/firewatch_150305_06.png" );
	g_renderer->SetModelMatrix( Mat44() );
	//g_renderer->BindTexture( texture );
	std::vector<Vertex_PCUTBN> vertices;

	AppendVertsForAABB2DWithDepth( vertices, AABB2( -1.f, -1.f, 1.f, 1.f ), -6.f, Rgba8::WHITE );
	g_renderer->DrawVertexArray( vertices );

	model = m_sphereMeshTransform.GetAsMatrix();
	g_renderer->SetModelMatrix( model, Rgba8::GREEN );
	g_renderer->DrawMesh( m_sphereMesh );
   
	g_renderer->EndCamera( *m_worldCamera );

	DebugRenderWorldToCamera( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets...", Rgba8::WHITE );

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	float deltaSeconds = (float)m_gameClock->GetLastDeltaSeconds();

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

	Transform transform = m_worldCamera->GetTransform();
	m_worldCamera->SetPitchRollYawRotation( transform.m_rotation.x + pitch * deltaSeconds,
											0.f,
											transform.m_rotation.z + yaw * deltaSeconds );

	// Translation
	TranslateCameraFPS( cameraTranslation * deltaSeconds );

	if ( g_inputSystem->WasKeyJustPressed( KEY_F2 ) )
	{
		g_renderer->CycleSampler();
	}
	if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
	{
		g_renderer->CycleBlendMode();
	}


	// Debug Commands
	if ( g_inputSystem->IsKeyPressed( 'Q' ) )
	{
		DebugAddWorldPoint( m_worldCamera->GetTransform().GetPosition(), .01f, Rgba8::GREEN, Rgba8::RED, 10.f, DEBUG_RENDER_XRAY );
	}
	if ( g_inputSystem->WasKeyJustPressed( 'O' ) )
	{
		DebugAddWorldLine( m_worldCamera->GetTransform().GetPosition(), Rgba8::RED, Rgba8::GREEN, 
						   m_cubeMeshTransform.GetPosition(), Rgba8::WHITE, Rgba8::BLACK, 
						   10.f );
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
	if ( g_inputSystem->WasKeyJustPressed( 'T' ) )
	{
		Mat44 textView = m_worldCamera->GetViewMatrix();
		InvertOrthoNormalMatrix( textView );
		DebugAddWorldText( textView, Vec2::ZERO, Rgba8::BLUE, Rgba8::RED, 35.f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, "Zero!" );
		DebugAddWorldText( textView, Vec2( .5f, .5f ), Rgba8::GREEN, Rgba8::RED, 35.f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, "Mid!" );
		DebugAddWorldTextf( textView, Vec2::ONE, Rgba8::RED, 35.f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, "One!", 12 );
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
		DebugAddScreenArrow( Vec2(250.f, 300.f), Rgba8::BLUE, Vec2( 1000.f, 12.f ), Rgba8::GREEN, Rgba8::RED, Rgba8::YELLOW, 3.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '4' ) )
	{
		DebugAddScreenQuad( AABB2( Vec2( 250.f, 12.f ), Vec2( 1000.f, 120.f ) ), Rgba8::GREEN, Rgba8::RED, 10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '5' ) )
	{
		Texture* texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/firewatch_150305_06.png" );
		DebugAddScreenTexturedQuad( AABB2( Vec2( 10.f, 10.f ), Vec2( 500.f, 500.f ) ), texture, Rgba8::WHITE, 10.f );
	}
	if ( g_inputSystem->WasKeyJustPressed( '6' ) )
	{
		DebugAddScreenTextf( Vec4( .5f, .75f, 10.f, -10.f ), Vec2( .5f, .5f ), 10.f, Rgba8::WHITE, 10.f, "Here is some text %d", 13 );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::LoadNewMap( const std::string& mapName )
{
	PTR_SAFE_DELETE( m_world );

	m_world = new World();
	m_world->BuildNewMap( mapName );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
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
void Game::DebugRender() const
{
	m_world->DebugRender();
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

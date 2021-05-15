#include "Game/Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UISystem.hpp"

#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Game/BufferTests.hpp"


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
	BufferTests::RunTests();

	m_worldCamera = new Camera();
	m_worldCamera->SetOutputSize( Vec2( g_windowWidth, g_windowHeight ) );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	m_worldCamera->SetPosition( Vec3( Vec2( g_windowWidth, g_windowHeight ) * .5f, 0.f ) );
	m_worldCamera->SetProjectionOrthographic( g_windowHeight );

	Vec2 windowDimensions = g_window->GetDimensions();

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( windowDimensions );
	m_uiCamera->SetPosition( Vec3( windowDimensions * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( windowDimensions.y );

	EnableDebugRendering();
	
	InitializeFPSHistory();

	m_debugInfoTextBox = new TextBox( *g_renderer, AABB2( Vec2::ZERO, Vec2( 200.f, 80.f ) ) );

	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	m_uiSystem = new UISystem();
	m_uiSystem->Startup( g_window, g_renderer );

	m_world = new World( m_gameClock );

	m_world->AddNewMap( "ConvexRaycast" );

	g_eventSystem->RegisterMethodEvent( "save_ghcs_file", "Usage: save_ghcs_file name=<name of file relative to Run/Data> Save the current convex scene as a .ghcs file.", eUsageLocation::EVERYWHERE, this, &Game::SaveConvexSceneToFile );
	g_eventSystem->RegisterMethodEvent( "load_ghcs_file", "Usage: load_ghcs_file name=<name of file relative to Run/Data> Load a convex scene from a .ghcs file. ", eUsageLocation::EVERYWHERE, this, &Game::LoadConvexSceneFromFile );

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::BeginFrame()
{
	
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	m_uiSystem->Shutdown();

	// Clean up member variables
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_debugInfoTextBox );
	PTR_SAFE_DELETE( m_uiCamera );
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
	switch ( m_gameState )
	{
		case eGameState::ATTRACT:
		{
			UpdateFromKeyboard();
		}
		break;

		case eGameState::PLAYING:
		{
			UpdateFromKeyboard();

			m_world->Update();
		}
		break;
	}

	UpdateCameras();
	UpdateMousePositions();
	UpdateFramesPerSecond();

	m_uiSystem->Update();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	switch ( m_gameState )
	{
		case eGameState::PLAYING:
		case eGameState::PAUSED:
		{
			m_world->Render();
			if ( m_isDebugRendering )
			{
				m_world->DebugRender();
			}
		}
		break;
	}

	g_renderer->EndCamera( *m_worldCamera );

	g_renderer->BeginCamera( *m_uiCamera );

	switch ( m_gameState )
	{
		case eGameState::ATTRACT:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "GeometricSandbox2D" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 550.f, 400.f ), 30.f, "Press Any Key to Start" );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;
	}

	m_uiSystem->Render();
	if ( m_isDebugRendering )
	{
		m_uiSystem->DebugRender();
	}

	RenderFPSCounter();
	RenderStats();

	g_renderer->EndCamera( *m_uiCamera );

	DebugRenderWorldToCamera( m_worldCamera );
	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void Game::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets..." );

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	if ( g_devConsole->IsOpen() )
	{
		return;
	}

	switch ( m_gameState )
	{
		case eGameState::ATTRACT:
		{
			if ( g_inputSystem->WasAnyKeyJustPressed() )
			{
				ChangeGameState( eGameState::PLAYING );
			}
		}
		break;

		case eGameState::PLAYING:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				g_eventSystem->FireEvent( "Quit" );
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
			{
				m_isDebugRendering = !m_isDebugRendering;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F2 ) )
			{
				m_world->CycleBroadphaseCheck();
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
			{
				m_world->CycleNarrowphaseCheck();
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F8 ) )
			{
				Entity::ResetEntityIds();
				m_world->ResetCurrentMap();
			}
		}
		case eGameState::PAUSED:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
			{
				g_eventSystem->FireEvent( "Quit" );
			}
		}
		break;

	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMousePositions()
{
	UpdateMouseWorldPosition();
	UpdateMouseUIPosition();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseWorldPosition()
{
	m_mouseWorldPosition = g_inputSystem->GetNormalizedMouseClientPos() * m_worldCamera->GetOutputSize();
	m_mouseWorldPosition += m_worldCamera->GetOrthoMin();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseUIPosition()
{
	m_mouseUIPosition = g_inputSystem->GetNormalizedMouseClientPos() * m_uiCamera->GetOutputSize();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	// World camera
	m_screenShakeIntensity -= SCREEN_SHAKE_ABLATION_PER_SECOND * (float)m_gameClock->GetLastDeltaSeconds();
	m_screenShakeIntensity = ClampZeroToOne( m_screenShakeIntensity );

	float maxScreenShake = m_screenShakeIntensity * MAX_CAMERA_SHAKE_DIST;
	float cameraShakeX = m_rng->RollRandomFloatInRange( -maxScreenShake, maxScreenShake );
	float cameraShakeY = m_rng->RollRandomFloatInRange( -maxScreenShake, maxScreenShake );
	Vec2 cameraShakeOffset = Vec2( cameraShakeX, cameraShakeY );

	//m_worldCamera->Translate2D( cameraShakeOffset );
	//m_worldCamera->SetPosition( m_focalPoint + Vec3( cameraShakeOffset, 0.f ) );
	//m_worldCamera->SetProjectionOrthographic( WINDOW_HEIGHT );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeFPSHistory()
{
	// Optimistically initialize fps history to 60 fps
	for ( int i = 0; i < FRAME_HISTORY_COUNT; ++i )
	{
		m_fpsHistory[i] = 60.f;
		m_fpsHistorySum += 60.f;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFramesPerSecond()
{
	float lastFPS = (float)m_gameClock->GetLastDeltaSeconds();

	float curFPS;

	if ( lastFPS < .0001f )
	{
		curFPS = 9999999.f;
	}
	else
	{
		curFPS = 1.f / lastFPS;
	}

	m_fpsHistorySum -= m_fpsHistory[m_fpsNextIdx];
	m_fpsHistory[m_fpsNextIdx] = curFPS;
	m_fpsHistorySum += curFPS;

	++m_fpsNextIdx;
	if ( m_fpsNextIdx >= FRAME_HISTORY_COUNT - 1 )
	{
		m_fpsNextIdx = 0;
	}
}


//-----------------------------------------------------------------------------------------------
float Game::GetAverageFPS() const
{
	constexpr float oneOverFrameCount = 1.f / (float)FRAME_HISTORY_COUNT;

	return m_fpsHistorySum * oneOverFrameCount;
}


//-----------------------------------------------------------------------------------------------
void Game::RenderFPSCounter() const
{
	float fps = GetAverageFPS();

	Rgba8 fpsCountercolor = Rgba8::GREEN;

	if ( fps < 30.f )
	{
		fpsCountercolor = Rgba8::RED;
	}
	if ( fps < 55.f )
	{
		fpsCountercolor = Rgba8::YELLOW;
	}

	float frameTime = (float)m_gameClock->GetLastDeltaSeconds() * 1000.f;

	DebugAddScreenTextf( Vec4( 0.75f, .97f, 0.f, 0.f ), Vec2::ZERO, 15.f, fpsCountercolor, 0.f,
						 "FPS: %.2f ( %.2f ms/frame )",
						 fps, frameTime );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderStats() const
{
	float screenHeight = .97f;
	float offsetBetweenLines = .02f;

	DebugAddScreenTextf( Vec4( 0.01f, screenHeight, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "Raycast Time: %.2f ms", m_world->GetRaycastTimeMs() );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "Num Raycast Impacts: %i", m_world->GetNumRaycastImpacts() );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[, .] Object Count: %i", m_world->GetObjectCount() );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[N M] Raycast Count: %i", m_world->GetRaycastCount() );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[S E] + [Mouse Drag] Adjust Visible Raycast Count Start and End" );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[W R] Rotate object" );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[K L] Scale object" );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[F2] Broadphase Check Type: %s", m_world->GetBroadphaseCheckTypeStr().c_str() );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[F3] Narrowphase Check Type: %s", m_world->GetNarrowphaseCheckTypeStr().c_str() );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[F1] Toggle debug drawing" );
	DebugAddScreenTextf( Vec4( 0.01f, ( screenHeight -= offsetBetweenLines ), 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::WHITE, 0.f, "[F8] Reload Game" );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::SetWorldCameraPosition( const Vec3& position )
{
	m_focalPoint = position;
}


//-----------------------------------------------------------------------------------------------
void Game::SetSceneCameras( const AABB2& sceneBounds )
{
	m_worldCamera->SetOutputSize( sceneBounds.GetDimensions() );
	m_worldCamera->SetPosition( Vec3( sceneBounds.GetDimensions() * .5f, 0.f ) );
	m_worldCamera->SetProjectionOrthographic( sceneBounds.GetHeight() );
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
void Game::ChangeGameState( const eGameState& newGameState )
{
	m_gameState = newGameState;
}


//-----------------------------------------------------------------------------------------------
void Game::SaveConvexSceneToFile( EventArgs* args )
{
	std::string fileName = args->GetValue( "name", "" );
	if ( fileName.empty() )
	{
		g_devConsole->PrintError( "Must specify name of file to save" );
		return;
	}

	std::string fileExtension = GetFileExtension( fileName );
	if ( fileExtension.empty() )
	{
		fileName.append( ".ghcs" );
	}
	else if ( IsEqualIgnoreCase( fileExtension, ".ghcs" ) )
	{
		// all good, don't need to change filename
	}
	else
	{
		g_devConsole->PrintError( "Convex scene files must have extension: .ghcs" );
		return;
	}

	m_world->SaveConvexSceneToFile( fileName );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadConvexSceneFromFile( EventArgs* args )
{
	std::string fileName = args->GetValue( "name", "" );
	if ( fileName.empty() )
	{
		g_devConsole->PrintError( "Must specify name of file to load (relative to Run/Data)" );
		return;
	}

	std::string fileExtension = GetFileExtension( fileName );
	if ( fileExtension.empty() )
	{
		fileName.append( ".ghcs" );
	}
	else if ( IsEqualIgnoreCase( fileExtension, ".ghcs" ) )
	{
		// all good, don't need to change filename
	}
	else
	{
		g_devConsole->PrintError( "Convex scene files must have extension: .ghcs" );
		return;
	}

	m_world->LoadConvexSceneFromFile( fileName );
}

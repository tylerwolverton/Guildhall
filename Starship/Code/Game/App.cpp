#include "App.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
App* g_app = nullptr;				    // Created and owned by Main_Windows.cpp
RenderContext* g_renderer = nullptr;		// Created and owned by the App
InputSystem* g_inputSystem = nullptr;			// Created and owned by the App
Window* g_window = nullptr;					// Owned by Main_Windows.cpp


//-----------------------------------------------------------------------------------------------
App::App()
{
	
}


//-----------------------------------------------------------------------------------------------
App::~App()
{
	
}


//-----------------------------------------------------------------------------------------------
void App::Startup()
{
	PopulateGameConfig();

	std::string windowTitle = g_gameConfigBlackboard.GetValue( "windowTitle", "Starship Gold" );
	float windowAspect = g_gameConfigBlackboard.GetValue( "windowAspect", 2.f );
	float windowHeightRatio = g_gameConfigBlackboard.GetValue( "windowHeightRatio", .9f );
	eWindowMode windowMode = GetWindowModeFromGameConfig();

	Clock::MasterStartup();

	g_window = new Window();
	g_window->Open( windowTitle, windowAspect, windowHeightRatio, windowMode );

	g_eventSystem = new EventSystem();
	g_devConsole = new DevConsole();
	g_inputSystem = new InputSystem();
	g_renderer = new RenderContext();
	m_theGame = new Game();
	
	g_eventSystem->Startup();
	g_window->SetEventSystem( g_eventSystem );

	g_inputSystem->Startup( g_window );
	g_window->SetInputSystem( g_inputSystem );

	g_renderer->Startup( g_window );

	g_devConsole->Startup();
	g_devConsole->SetInputSystem( g_inputSystem );
	g_devConsole->SetRenderer( g_renderer );
	g_devConsole->SetBitmapFont( g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" ) );

	m_theGame->Startup();
}


//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	m_theGame->Shutdown();
	g_renderer->Shutdown();
	g_inputSystem->Shutdown();
	g_devConsole->Shutdown();
	g_inputSystem->Shutdown();
	g_eventSystem->Shutdown();
	g_window->Close();

	Clock::MasterShutdown();

	PTR_SAFE_DELETE( m_theGame );
	PTR_SAFE_DELETE( g_devConsole );
	PTR_SAFE_DELETE( g_renderer );
	PTR_SAFE_DELETE( g_inputSystem );
	PTR_SAFE_DELETE( g_eventSystem );
	PTR_SAFE_DELETE( g_window );
}


//-----------------------------------------------------------------------------------------------
void App::RunFrame()
{
	static double timeLastFrameStarted = GetCurrentTimeSeconds(); // Runs once only!	
	double timeThisFrameStarted = GetCurrentTimeSeconds();
	double deltaSeconds = timeThisFrameStarted - timeLastFrameStarted;
	timeLastFrameStarted = timeThisFrameStarted;

	BeginFrame();											// for all engine systems (NOT the game)
	Update( ClampMinMax( (float)deltaSeconds, 0.f, 0.1f) ); // for the game only
	Render();												// for the game only
	EndFrame();												// for all engine systems (NOT the game)
}


//-----------------------------------------------------------------------------------------------
bool App::HandleQuitRequested()
{
	m_isQuitting = true;

	return 0;
}


//-----------------------------------------------------------------------------------------------
void App::RestartGame()
{
	m_theGame->Shutdown();
	delete m_theGame;

	m_theGame = new Game();
	m_theGame->Startup();
}


//-----------------------------------------------------------------------------------------------
eWindowMode App::GetWindowModeFromGameConfig()
{
	std::string windowModeStr = g_gameConfigBlackboard.GetValue( "windowMode", "windowed" );

	if ( !_strcmpi( windowModeStr.c_str(), "windowed" ) )
	{
		return eWindowMode::WINDOWED;
	}
	else if ( !_strcmpi( windowModeStr.c_str(), "borderless" ) )
	{
		return eWindowMode::BORDERLESS;
	}
	else
	{
		//g_devConsole->PrintWarning( Stringf( "Unrecognized window mode '%s' found in game config; using windowed mode.", windowModeStr.c_str() ) );
		return eWindowMode::WINDOWED;
	}
}


//-----------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	Clock::MasterBeginFrame();

	g_window->BeginFrame();
	g_eventSystem->BeginFrame();
	g_devConsole->BeginFrame();
	g_inputSystem->BeginFrame();
	g_renderer->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
	g_devConsole->Update();
	UpdateFromKeyboard( deltaSeconds );

	// Modify deltaSeconds based on game state
	if( m_isPaused )
	{
		deltaSeconds = 0.f;
	}
	if( m_isSlowMo )
	{
		deltaSeconds *= .1f;
	}

	m_theGame->Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void App::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	
	if ( g_inputSystem->WasKeyJustPressed( 'P' ) )
	{
		m_isPaused = !m_isPaused;
	}
	
	if ( g_inputSystem->IsKeyPressed( 'T' ) )
	{
		m_isSlowMo = true;
	}
	else
	{
		m_isSlowMo = false;
	}
	
	if ( g_inputSystem->WasKeyJustPressed( 'N' ) )
	{
		m_theGame->RespawnPlayerIfDead();
	}
	
	if ( g_inputSystem->WasKeyJustPressed( 'O' ) )
	{
		m_theGame->SpawnAsteroid();
	}
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
	{
		HandleQuitRequested();
	}
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
	{
		m_isDebugRendering = !m_isDebugRendering;
	}
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_F8 ) )
	{
		g_app->RestartGame();
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SPACEBAR ) )
	{
		m_theGame->HandleBulletFired();
	}
}


//-----------------------------------------------------------------------------------------------
void App::Render() const
{
	m_theGame->Render();
	if ( m_isDebugRendering )
	{
		m_theGame->DebugRender();
	}

	g_devConsole->Render();
}


//-----------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_renderer->EndFrame();
	g_inputSystem->EndFrame();
	g_devConsole->EndFrame();
	g_eventSystem->EndFrame();
	g_window->EndFrame();
}

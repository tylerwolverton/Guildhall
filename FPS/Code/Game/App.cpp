#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


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

	std::string windowTitle = g_gameConfigBlackboard.GetValue( "windowTitle", "SD2.A01" );
	float windowAspect = g_gameConfigBlackboard.GetValue( "windowAspect", 16.f / 9.f );
	float windowHeightRatio = g_gameConfigBlackboard.GetValue( "windowHeightRatio", .9f );
	eWindowMode windowMode = GetWindowModeFromGameConfig();

	g_window = new Window();
	g_window->Open( windowTitle, windowAspect, windowHeightRatio, windowMode );

	g_eventSystem = new EventSystem();
	g_devConsole = new DevConsole();
	g_inputSystem = new InputSystem();
	g_audioSystem = new AudioSystem();
	g_renderer = new RenderContext();
	g_game = new Game();

	g_eventSystem->Startup();
	g_window->SetEventSystem( g_eventSystem );

	g_devConsole->Startup();
	
	g_inputSystem->Startup();
	g_window->SetInputSystem( g_inputSystem );

	g_audioSystem->Startup();
	g_renderer->Startup( g_window );
	g_game->Startup();

	g_eventSystem->RegisterEvent( "QuitGame", QuitGame );
}


//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	g_game->Shutdown();
	g_renderer->Shutdown();
	g_audioSystem->Shutdown();
	g_inputSystem->Shutdown();
	g_devConsole->Shutdown();
	g_eventSystem->Shutdown();
	g_window->Close();

	delete g_game;
	g_game = nullptr;

	delete g_renderer;
	g_renderer = nullptr;

	delete g_audioSystem;
	g_audioSystem = nullptr;

	delete g_inputSystem;
	g_inputSystem = nullptr;

	delete g_devConsole;
	g_devConsole = nullptr;

	delete g_eventSystem;
	g_eventSystem = nullptr;

	delete g_window;
	g_window = nullptr;
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
	g_game->Shutdown();
	delete g_game;

	g_game = new Game();
	g_game->Startup();
}


//-----------------------------------------------------------------------------------------------
void App::PopulateGameConfig()
{
	XmlDocument doc;
	XmlError loadError = doc.LoadFile( "Data/GameConfig.xml" );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		return;
	}

	XmlElement* root = doc.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes( *root );
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
		g_devConsole->PrintString( Rgba8::YELLOW, Stringf( "Unrecognized window mode '%s' found in game config; using windowed mode.", windowModeStr.c_str() ) );
		return eWindowMode::WINDOWED;
	}
}


//-----------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	g_window->BeginFrame();
	g_eventSystem->BeginFrame();
	g_devConsole->BeginFrame();
	g_inputSystem->BeginFrame();
	g_audioSystem->BeginFrame();
	g_renderer->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
	UpdateFromKeyboard( deltaSeconds );

	g_renderer->UpdateFrameTime( deltaSeconds );
	g_game->Update( deltaSeconds );
	g_devConsole->Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void App::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
	{
		HandleQuitRequested();
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F8 ) )
	{
		g_app->RestartGame();
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_TILDE ) )
	{
		g_devConsole->ToggleOpenFull();
	}

	if ( g_devConsole->IsOpen() )
	{
		if ( g_inputSystem->WasKeyJustPressed( KEY_RIGHTARROW ) )
		{
			g_devConsole->InsertCharacterIntoCommand( "a" );
			//g_devConsole->MoveCursorPosition( 1 );
		}
		if ( g_inputSystem->WasKeyJustPressed( KEY_LEFTARROW ) )
		{
			g_devConsole->MoveCursorPosition( -1 );
		}
	
	}
}


//-----------------------------------------------------------------------------------------------
void App::Render() const
{
	g_game->Render();
	g_devConsole->Render( *g_renderer, AABB2( -1.f, -1.f, 1.f, 1.f ), .05f );
}


//-----------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_renderer->EndFrame();
	g_audioSystem->EndFrame();
	g_inputSystem->EndFrame();
	g_devConsole->EndFrame();
	g_eventSystem->EndFrame();
	g_window->EndFrame();
}


//-----------------------------------------------------------------------------------------------
bool App::QuitGame( EventArgs args )
{
	g_app->HandleQuitRequested();

	return 0;
}

#include "App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
App* g_app = nullptr;				    // Created and owned by Main_Windows.cpp
RenderContext* g_renderer = nullptr;	// Created and owned by the App
InputSystem* g_inputSystem = nullptr;	// Created and owned by the App
AudioSystem* g_audioSystem = nullptr;	// Created and owned by the App
Game* g_game = nullptr;					// Created and owned by the App


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
	g_inputSystem = new InputSystem();
	g_audioSystem = new AudioSystem();
	g_renderer = new RenderContext();
	g_game = new Game();
	
	g_inputSystem->Startup();
	g_audioSystem->Startup();
	g_renderer->Startup();
	g_game->Startup();
}


//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	g_game->Shutdown();
	g_renderer->Shutdown();
	g_audioSystem->Shutdown();
	g_inputSystem->Shutdown();
	
	delete g_game;
	g_game = nullptr;
	
	delete g_renderer;
	g_renderer = nullptr;

	delete g_audioSystem;
	g_audioSystem = nullptr;
	
	delete g_inputSystem;
	g_inputSystem = nullptr;
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
	g_game->StopAllMusic();
	g_game->Shutdown();
	delete g_game;

	g_game = new Game();
	g_game->Startup();
}


//-----------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	g_inputSystem->BeginFrame();
	g_audioSystem->BeginFrame();
	g_renderer->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
	UpdateFromKeyboard( deltaSeconds );

	g_game->Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void App::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_F8 ) )
	{
		g_app->RestartGame();
	}
}


//-----------------------------------------------------------------------------------------------
void App::Render() const
{
	g_game->Render();
}


//-----------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_renderer->EndFrame();
	g_audioSystem->EndFrame();
	g_inputSystem->EndFrame();
}

#include "App.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
App* g_app = nullptr;				    // Created and owned by Main_Windows.cpp
RenderContext* g_renderer = nullptr;		// Created and owned by the App
InputSystem* g_inputSystem = nullptr;			// Created and owned by the App


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
	g_renderer = new RenderContext();
	m_theGame = new Game();
	
	g_inputSystem->Startup();
	g_renderer->Startup();
	m_theGame->Startup();
}


//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	m_theGame->Shutdown();
	g_renderer->Shutdown();
	g_inputSystem->Shutdown();
	
	delete m_theGame;
	m_theGame = nullptr;
	
	delete g_renderer;
	g_renderer = nullptr;

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
	m_theGame->Shutdown();
	delete m_theGame;

	m_theGame = new Game();
	m_theGame->Startup();
}


//-----------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	g_inputSystem->BeginFrame();
	g_renderer->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
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
}


//-----------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_renderer->EndFrame();
	g_inputSystem->EndFrame();
}

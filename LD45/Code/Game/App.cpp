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
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
App* g_theApp = nullptr;				    // Created and owned by Main_Windows.cpp
RenderContext* g_theRenderer = nullptr;		// Created and owned by the App
InputSystem* g_theInput = nullptr;			// Created and owned by the App


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
	g_theInput = new InputSystem();
	g_theRenderer = new RenderContext();
	m_theGame = new Game();
	
	g_theInput->Startup();
	g_theRenderer->Startup();
	m_theGame->Startup();
}


//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	m_theGame->Shutdown();
	g_theRenderer->Shutdown();
	g_theInput->Shutdown();
	
	delete m_theGame;
	m_theGame = nullptr;
	
	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theInput;
	g_theInput = nullptr;
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
	g_theInput->BeginFrame();
	g_theRenderer->BeginFrame();
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

	const KeyButtonState* keyboardState = g_theInput->GetKeyboardState();

	if ( keyboardState[KeyCode::KEY_CODE_ID_ESC].WasJustPressed() )
	{
		HandleQuitRequested();
	}

	if ( keyboardState[KeyCode::KEY_CODE_ID_F1].WasJustPressed() )
	{
		m_theGame->SetDebugRendering( !m_theGame->GetDebugRendering() );
	}

	if ( keyboardState[KeyCode::KEY_CODE_ID_F8].WasJustPressed() )
	{
		g_theApp->RestartGame();
	}

	if ( keyboardState[KeyCode::KEY_CODE_ID_P].WasJustPressed() )
	{
		m_isPaused = !m_isPaused;
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
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
}

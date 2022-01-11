#include "Editor/Editor.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"

#include "Editor/EditorCommon.hpp"


//-----------------------------------------------------------------------------------------------
Editor::Editor()
{
} 


//-----------------------------------------------------------------------------------------------
Editor::~Editor()
{
}


//-----------------------------------------------------------------------------------------------
void Editor::Startup()
{
	m_worldCamera = new Camera();
	m_worldCamera->SetOutputSize( Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	m_worldCamera->SetPosition( m_focalPoint );

	Vec2 windowDimensions = g_window->GetDimensions();

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( windowDimensions );
	m_uiCamera->SetPosition( Vec3( windowDimensions * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( windowDimensions.y );

	EnableDebugRendering();
	
	InitializeFPSHistory();

	m_rng = new RandomNumberGenerator();

	m_editorClock = new Clock();

	g_renderer->Setup( m_editorClock );

	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	g_devConsole->PrintString( "Editor Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Editor::BeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
void Editor::Shutdown()
{
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_uiCamera );
	PTR_SAFE_DELETE( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Editor::RestartEditor()
{
	Shutdown();
	Startup();
}


//-----------------------------------------------------------------------------------------------
void Editor::Update()
{
	switch ( m_editorState )
	{
		case eEditorState::LOADING:
		{
			switch ( m_loadingFrameNum )
			{
				case 0:
				{
					++m_loadingFrameNum;
				}
				break;

				case 1:
				{
					LoadAssets();
					ChangeEditorState( eEditorState::PLAYING );
					Update();
				}
				break;
			}
		}
		break;

		case eEditorState::PAUSED:
		{
			UpdateFromKeyboard();
		}
		break;

		case eEditorState::PLAYING:
		{
			UpdateFromKeyboard();
		}
		break;
	}

	UpdateCameras();
	UpdateMousePositions();
	UpdateFramesPerSecond();
}


//-----------------------------------------------------------------------------------------------
void Editor::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	switch ( m_editorState )
	{
		case eEditorState::PLAYING:
		case eEditorState::PAUSED:
		{
			//m_world->Render();
			if ( m_isDebugRendering )
			{
				//m_world->DebugRender();
			}
		}
		break;
	}

	g_renderer->EndCamera( *m_worldCamera );

	g_renderer->BeginCamera( *m_uiCamera );

	switch ( m_editorState )
	{
		case eEditorState::LOADING:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 450.f, 500.f ), 100.f, "Loading..." );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;
				
		case eEditorState::PAUSED:
		{
			std::vector<Vertex_PCU> vertexes;
			DrawAABB2( g_renderer, AABB2( Vec2::ZERO, Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) ), Rgba8( 0, 0, 0, 100 ) );

			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "Paused" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 550.f, 400.f ), 30.f, "Esc to Quit" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 550.f, 350.f ), 30.f, "Any Other Key to Resume" );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;
	}

	RenderFPSCounter();

	g_renderer->EndCamera( *m_uiCamera );

	DebugRenderWorldToCamera( m_worldCamera );
	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void Editor::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
void Editor::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets..." );


	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Editor::ReloadEditor()
{
	//m_world->Reset();

	g_editorConfigBlackboard.Clear();
	PopulateEditorConfig();

	PTR_VECTOR_SAFE_DELETE( SpriteSheet::s_definitions );

	EventArgs args;
	g_eventSystem->FireEvent( "OnEditorStart", &args );
	g_devConsole->PrintString( "Data files reloaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Editor::UpdateFromKeyboard()
{
	if ( g_devConsole->IsOpen() )
	{
		return;
	}

	switch ( m_editorState )
	{
		case eEditorState::PLAYING:
		{
			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_ESC ) )
			{
				ChangeEditorState( eEditorState::PAUSED );
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
			{
				m_isDebugRendering = !m_isDebugRendering;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F2 ) )
			{
				g_eventSystem->FireEvent( "TestEvent" );
			}

			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_F5 ) )
			{
				ReloadEditor();
			}

		}
		break;

		case eEditorState::PAUSED:
		{
			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_ESC ) )
			{
				g_eventSystem->FireEvent( "Quit" );
			}

			if ( g_inputSystem->ConsumeAnyKeyJustPressed() )
			{
				ChangeEditorState( eEditorState::PLAYING );
			}
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void Editor::UpdateMousePositions()
{
	UpdateMouseWorldPosition();
	UpdateMouseUIPosition();
}


//-----------------------------------------------------------------------------------------------
void Editor::UpdateMouseWorldPosition()
{
	m_mouseWorldPosition = g_inputSystem->GetNormalizedMouseClientPos() * m_worldCamera->GetOutputSize();
	m_mouseWorldPosition += m_worldCamera->GetOrthoMin();
}


//-----------------------------------------------------------------------------------------------
void Editor::UpdateMouseUIPosition()
{
	m_mouseUIPosition = g_inputSystem->GetNormalizedMouseClientPos() * m_uiCamera->GetOutputSize();
}


//-----------------------------------------------------------------------------------------------
void Editor::UpdateCameras()
{
	// World camera
	m_worldCamera->SetPosition( m_focalPoint );
	m_worldCamera->SetProjectionOrthographic( WINDOW_HEIGHT );
}


//-----------------------------------------------------------------------------------------------
void Editor::DebugRender() const
{
	//m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Editor::SetWorldCameraPosition( const Vec3& position )
{
	m_focalPoint = position;
}


//-----------------------------------------------------------------------------------------------
void Editor::ChangeEditorState( const eEditorState& newGameState )
{
	eEditorState oldGameState = m_editorState;
	m_editorState = newGameState;

	switch ( newGameState )
	{
		case eEditorState::LOADING:
		{
			ERROR_AND_DIE( "Tried to go back to the loading state during the game. Don't do that." );
		}
		break;

		case eEditorState::PLAYING:
		{
			m_editorClock->Resume();

			// Check which state we are changing from
			switch ( oldGameState )
			{
				case eEditorState::PAUSED:
				{
					m_editorClock->Resume();
				}
				break;
			}
		}
		break;

		case eEditorState::PAUSED:
		{
			m_editorClock->Pause();
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void Editor::InitializeFPSHistory()
{
	// Optimistically initialize fps history to 60 fps
	for ( int i = 0; i < FRAME_HISTORY_COUNT; ++i )
	{
		m_fpsHistory[i] = 60.f;
		m_fpsHistorySum += 60.f;
	}
}


//-----------------------------------------------------------------------------------------------
void Editor::UpdateFramesPerSecond()
{
	if ( m_editorClock->IsPaused() )
	{
		return;
	}

	float curFPS = 1.f / (float)m_editorClock->GetLastDeltaSeconds();

	if ( curFPS < 0 )
	{
		curFPS = 0;
	}
	else if ( curFPS > 99999.f )
	{
		curFPS = 99999.f;
	}

	m_fpsHistorySum -= m_fpsHistory[m_fpsNextIdx];
	m_fpsHistory[m_fpsNextIdx] = curFPS;
	m_fpsHistorySum += curFPS;

	++m_fpsNextIdx;
	if ( m_fpsNextIdx >= FRAME_HISTORY_COUNT )
	{
		m_fpsNextIdx = 0;
	}
}


//-----------------------------------------------------------------------------------------------
float Editor::GetAverageFPS() const
{
	constexpr float oneOverFrameCount = 1.f / (float)FRAME_HISTORY_COUNT;

	return m_fpsHistorySum * oneOverFrameCount;
}


//-----------------------------------------------------------------------------------------------
void Editor::RenderFPSCounter() const
{
	if ( m_editorClock->IsPaused() )
	{
		DebugAddScreenTextf( Vec4( 0.75f, .97f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::YELLOW, 0.f,
							 "Game Paused" );

		return;
	}

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

	float frameTime = (float)m_editorClock->GetLastDeltaSeconds() * 1000.f;

	DebugAddScreenTextf( Vec4( 0.75f, .97f, 0.f, 0.f ), Vec2::ZERO, 15.f, fpsCountercolor, 0.f,
						 "FPS: %.2f ( %.2f ms/frame )",
						 fps, frameTime );
}


//-----------------------------------------------------------------------------------------------
float Editor::GetLastDeltaSecondsf()
{
	return (float)m_editorClock->GetLastDeltaSeconds();
}

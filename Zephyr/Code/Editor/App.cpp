#include "Editor/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Time/Clock.hpp"

#include "ThirdParty/DearImgui/imgui.h"
#include "ThirdParty/DearImgui/imgui_impl_win32.h"
#include "ThirdParty/DearImgui/imgui_impl_dx11.h"

#include "Editor/EditorCommon.hpp"
#include "Editor/Editor.hpp"


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
	PopulateEditorConfig();

	std::string windowTitle = g_editorConfigBlackboard.GetValue( "windowTitle", "LevelEditor" );
	float windowAspect = g_editorConfigBlackboard.GetValue( "windowAspect", 16.f / 9.f );
	float windowHeightRatio = g_editorConfigBlackboard.GetValue( "windowHeightRatio", .9f );
	eWindowMode windowMode = GetWindowModeFromGameConfig();

	Clock::MasterStartup();

	g_window = new Window();
	g_window->Open( windowTitle, windowAspect, windowHeightRatio, windowMode );

	g_eventSystem = new EventSystem();
	g_inputSystem = new InputSystem();
	g_renderer = new RenderContext();
	g_devConsole = new DevConsole();
	g_editor = new Editor();

	g_eventSystem->Startup();
	g_window->SetEventSystem( g_eventSystem );

	g_inputSystem->Startup( g_window );
	g_window->SetInputSystem( g_inputSystem );

	g_renderer->Startup( g_window );
	DebugRenderSystemStartup( g_renderer, g_eventSystem );

	g_devConsole->Startup();
	g_devConsole->SetInputSystem( g_inputSystem );
	g_devConsole->SetRenderer( g_renderer );
	g_devConsole->SetBitmapFont( g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" ) );
	
	// TEMP DearImgui Initialization
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.DisplaySize.x = g_window->GetClientWidth();
	io.DisplaySize.y = g_window->GetClientHeight();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplWin32_Init( g_window->m_hwnd );
	ImGui_ImplDX11_Init( g_renderer->m_device, g_renderer->m_context );

	g_editor->Startup();

	g_eventSystem->RegisterEvent( "Quit", "Quit the game.", eUsageLocation::EVERYWHERE, QuitGame );
}


//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	g_editor->Shutdown();
	g_devConsole->Shutdown();
	DebugRenderSystemShutdown();
	g_renderer->Shutdown();
	g_inputSystem->Shutdown();
	g_eventSystem->Shutdown();
	g_window->Close();

	Clock::MasterShutdown();
		
	PTR_SAFE_DELETE( g_editor );
	PTR_SAFE_DELETE( g_devConsole );
	PTR_SAFE_DELETE( g_renderer );
	PTR_SAFE_DELETE( g_inputSystem );
	PTR_SAFE_DELETE( g_eventSystem );
	PTR_SAFE_DELETE( g_window );
}


//-----------------------------------------------------------------------------------------------
void App::RunFrame()
{
	BeginFrame();											// for all engine systems (NOT the game)
	Update();												// for the game only
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
void App::RestartEditor()
{
	g_editor->Shutdown();
	delete g_editor;

	g_editor = new Editor();
	g_editor->Startup();
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
	DebugRenderBeginFrame();
	g_editor->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
void App::Update()
{
	g_devConsole->Update();
	g_editor->Update();

	UpdateFromKeyboard();
}


//-----------------------------------------------------------------------------------------------
void App::UpdateFromKeyboard()
{
	if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
	{
		if ( g_devConsole->IsOpen() )
		{
			g_devConsole->Close();
			g_inputSystem->ConsumeAnyKeyJustPressed();
		}
	}
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_TILDE ) )
	{
		g_devConsole->ToggleOpenFull();
		g_inputSystem->ConsumeAnyKeyJustPressed();
	}
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_F11 ) )
	{
		g_window->ToggleWindowMode();
	}
}


//-----------------------------------------------------------------------------------------------
void App::Render() const
{
	g_editor->Render();
	g_devConsole->Render();
}


//-----------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_editor->EndFrame();
	DebugRenderEndFrame();
	g_renderer->EndFrame();
	g_inputSystem->EndFrame();
	g_devConsole->EndFrame();
	g_eventSystem->EndFrame();
	g_window->EndFrame();
}


//-----------------------------------------------------------------------------------------------
eWindowMode App::GetWindowModeFromGameConfig()
{
	std::string windowModeStr = g_editorConfigBlackboard.GetValue( "windowMode", "windowed" );

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
		g_devConsole->PrintWarning( Stringf( "Unrecognized window mode '%s' found in game config; using windowed mode.", windowModeStr.c_str() ) );
		return eWindowMode::WINDOWED;
	}
}


//-----------------------------------------------------------------------------------------------
bool App::QuitGame( EventArgs* args )
{
	UNUSED( args );
	g_app->HandleQuitRequested();

	return 0;
}

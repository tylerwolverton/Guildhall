#include "Engine/DearImgui/DearImguiSystem.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "ThirdParty/DearImgui/imgui.h"
#include "ThirdParty/DearImgui/imgui_impl_win32.h"
#include "ThirdParty/DearImgui/imgui_impl_dx11.h"


//-----------------------------------------------------------------------------------------------
void DearImguiSystem::Startup( const RenderContext& renderer, const Window& window )
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.DisplaySize.x = (float)window.GetClientWidth();
	io.DisplaySize.y = (float)window.GetClientHeight();
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

	ImGui_ImplWin32_Init( window.m_hwnd );
	ImGui_ImplDX11_Init( renderer.m_device, renderer.m_context );
}


//-----------------------------------------------------------------------------------------------
void DearImguiSystem::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


//-----------------------------------------------------------------------------------------------
void DearImguiSystem::Render()
{
	bool temp = true;
	ImGui::ShowDemoWindow( &temp );

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}


//-----------------------------------------------------------------------------------------------
void DearImguiSystem::BeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}


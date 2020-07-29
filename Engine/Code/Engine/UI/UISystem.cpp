#include "Engine/UI/UISystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/UI/UIPanel.hpp"


//-----------------------------------------------------------------------------------------------
void UISystem::Startup( Window* window, RenderContext* renderer )
{
	m_renderer = renderer;
	m_inputSystem = window->GetInputSystem();
	m_windowDimensions = window->GetDimensions();

	m_rootPanel = new UIPanel( *this, AABB2( Vec2::ZERO, m_windowDimensions ) );
}


//-----------------------------------------------------------------------------------------------
void UISystem::Update()
{
	m_rootPanel->Update();
}


//-----------------------------------------------------------------------------------------------
void UISystem::Render()
{
	m_rootPanel->Render();
}


//-----------------------------------------------------------------------------------------------
void UISystem::DebugRender()
{
	m_rootPanel->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void UISystem::Shutdown()
{
	PTR_SAFE_DELETE( m_rootPanel );
}

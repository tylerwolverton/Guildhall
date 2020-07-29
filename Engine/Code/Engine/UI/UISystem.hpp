#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class InputSystem;
class RenderContext;
class UIPanel;
class Window;


//-----------------------------------------------------------------------------------------------
class UISystem
{
	friend class UIButton;
	friend class UIElement;
	friend class UIPanel;
	friend class UIButton;
	friend class UILabel;
	friend class UIImage;
	friend class UIText;

public:
	void Startup( Window* window, RenderContext* renderer );
	void Update();
	void Render();
	void DebugRender();
	void Shutdown();

	UIPanel* GetRootPanel()										{ return m_rootPanel; }

private:
	RenderContext*	m_renderer = nullptr;
	InputSystem*	m_inputSystem = nullptr;
	Vec2			m_windowDimensions;

	UIPanel*		m_rootPanel = nullptr;
};

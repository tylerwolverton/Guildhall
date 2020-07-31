#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class InputSystem;
class RenderContext;
class UIPanel;
class Window;


//-----------------------------------------------------------------------------------------------
// Define the size and alignment of an element within its parent
//-----------------------------------------------------------------------------------------------
struct UIAlignedPositionData
{
public:
	Vec2 fractionOfParentDimensions = Vec2::ONE;
	Vec2 horizontalPaddingInPixels = Vec2::ZERO;
	Vec2 verticalPaddingInPixels = Vec2::ZERO;
	Vec2 positionOffsetInPixels = Vec2::ZERO;
	Vec2 alignmentWithinParentElement = ALIGN_CENTERED;
};


//-----------------------------------------------------------------------------------------------
// Define the relative location of an element within its parent
//-----------------------------------------------------------------------------------------------
struct UIRelativePositionData
{
public:
	Vec2 widthFractionRange = Vec2::ZERO_TO_ONE;
	Vec2 heightFractionRange = Vec2::ZERO_TO_ONE;
	Vec2 horizontalPaddingInPixels = Vec2::ZERO;
	Vec2 verticalPaddingInPixels = Vec2::ZERO;
	Vec2 positionOffsetInPixels = Vec2::ZERO;
};


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

	UIPanel* const GetRootPanel()										{ return m_rootPanel; }

private:
	RenderContext*	m_renderer = nullptr;
	InputSystem*	m_inputSystem = nullptr;
	Vec2			m_windowDimensions;

	UIPanel*		m_rootPanel = nullptr;
};

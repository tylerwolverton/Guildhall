#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/UI/UIElement.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class InputSystem;
class SpriteDefinition;
class Texture;
class UIPanel;
class UILabel;


//-----------------------------------------------------------------------------------------------
class UIButton : public UIElement
{
	friend class UIPanel;
	friend class UIUniformGrid;

public:
	virtual ~UIButton();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	Vec2 GetPosition() const;

private:
	UIButton( UISystem& uiSystem, const AABB2& absoluteScreenBounds, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE, const std::string& name = "" );
	UIButton( UISystem& uiSystem, const UIPanel& parentPanel, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE, const std::string& name = "" );
	UIButton( UISystem& uiSystem, const UIPanel& parentPanel, const UIAlignedPositionData& positionData, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE, const std::string& name = "" );
	UIButton( UISystem& uiSystem, const UIPanel& parentPanel, const UIRelativePositionData& positionData, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE, const std::string& name = "" );

public:
	Delegate<EventArgs*> m_onClickEvent;
	Delegate<EventArgs*> m_onReleaseEvent;
	Delegate<EventArgs*> m_onHoverBeginEvent;
	Delegate<EventArgs*> m_onHoverStayEvent;
	Delegate<EventArgs*> m_onHoverEndEvent;

private:	
	bool m_isMouseHovering = false;
};

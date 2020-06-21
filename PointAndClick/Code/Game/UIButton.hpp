#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/UIElement.hpp"

#include <vector>

//-----------------------------------------------------------------------------------------------
class InputSystem;
class RenderContext;
class SpriteDefinition;
class Texture;
class UIPanel;
class UILabel;


//-----------------------------------------------------------------------------------------------
class UIButton : public UIElement
{
public:
	UIButton( const AABB2& absoluteScreenBounds, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	UIButton( const UIPanel& parentPanel, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	~UIButton();

	virtual void Update() override;
	virtual void Render( RenderContext* renderer ) const override;

	Vec2 GetPosition() const;

	UILabel* AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions,
					   Texture* image = nullptr );
	UILabel* AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions,
					   SpriteDefinition* spriteDef = nullptr );
	UILabel* AddText( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions,
					  const std::string& text );

	void	 ClearLabels();

public:
	Delegate<EventArgs*> m_onClickEvent;
	Delegate<EventArgs*> m_onHoverBeginEvent;
	Delegate<EventArgs*> m_onHoverStayEvent;
	Delegate<EventArgs*> m_onHoverEndEvent;

private:	
	bool m_isMouseHovering = false;
	
	std::vector<UILabel*> m_labels;
};

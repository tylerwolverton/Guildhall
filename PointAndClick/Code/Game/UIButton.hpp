#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class InputSystem;
class RenderContext;
class Texture;
class UIPanel;


//-----------------------------------------------------------------------------------------------
class UIButton
{
public:
	UIButton( const AABB2& absoluteScreenBounds, Texture* backgroundTexture = nullptr );
	UIButton( const UIPanel& parentPanel, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture = nullptr );
	~UIButton();

	void Update();
	void Render( RenderContext* renderer ) const;

	void Activate();
	void Deactivate();
	void Hide();
	void Show();

	void SetTint( const Rgba8& tint )									{ m_tint = tint; }

	Vec2 GetPosition();

public:
	Delegate<EventArgs*> m_onClickEvent;
	Delegate<EventArgs*> m_onHoverBeginEvent;
	Delegate<EventArgs*> m_onHoverEndEvent;

private:
	// uint m_id;
	bool m_isActive = true;
	bool m_isVisible = true;
	
	bool m_isMouseHovering = false;

	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;

	Texture* m_backgroundTexture = nullptr;
};
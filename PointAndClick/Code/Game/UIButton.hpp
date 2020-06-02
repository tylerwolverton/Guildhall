#pragma once
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class InputSystem;
class RenderContext;
class Texture;


//-----------------------------------------------------------------------------------------------
class UIButton
{
public:
	UIButton( const AABB2& absoluteScreenBounds, Texture* backgroundTexture = nullptr );
	UIButton( const Vec2& dimensions, const Vec2& position, Texture* backgroundTexture = nullptr );
	~UIButton();

	void Update();
	void Render( RenderContext* renderer ) const;

	void Activate();
	void Deactivate();
	void Hide();
	void Show();

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

	Texture* m_backgroundTexture = nullptr;
};

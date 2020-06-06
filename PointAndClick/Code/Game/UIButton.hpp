#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>

//-----------------------------------------------------------------------------------------------
class InputSystem;
class RenderContext;
class Texture;
class UIPanel;
class UILabel;


//-----------------------------------------------------------------------------------------------
class UIButton
{
public:
	UIButton( const AABB2& absoluteScreenBounds, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	UIButton( const UIPanel& parentPanel, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	~UIButton();

	void Update();
	void Render( RenderContext* renderer ) const;

	void Activate();
	void Deactivate();
	void Hide();
	void Show();

	void SetTint( const Rgba8& tint )									{ m_tint = tint; }

	uint GetId() const													{ return m_id; }
	Vec2 GetPosition() const;
	AABB2 GetBoundingBox() const										{ return m_boundingBox; }

	UILabel* AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions,
					   Texture* image = nullptr );
	UILabel* AddText( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions,
					  const std::string& text );

public:
	Delegate<EventArgs*> m_onClickEvent;
	Delegate<EventArgs*> m_onHoverBeginEvent;
	Delegate<EventArgs*> m_onHoverEndEvent;

private:
	uint m_id = 0;
	bool m_isActive = true;
	bool m_isVisible = true;
	
	bool m_isMouseHovering = false;

	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;

	Texture* m_backgroundTexture = nullptr;

	std::vector<UILabel*> m_labels;
};

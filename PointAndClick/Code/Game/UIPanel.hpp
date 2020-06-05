#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec4.hpp"

#include "Game/UIButton.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class RenderContext;
class Texture;


//-----------------------------------------------------------------------------------------------
class UIPanel
{
public:
	UIPanel( const AABB2& absoluteScreenBounds, Texture* backgroundTexture = nullptr );
	UIPanel( UIPanel* parentPanel, const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture = nullptr );
	~UIPanel();

	void Update();
	void Render( RenderContext* renderer ) const;

	void Activate();
	void Deactivate();
	void Hide();
	void Show();

	void SetBackgroundTexture( Texture* backgroundTexture )							{ m_backgroundTexture = backgroundTexture; }
	void SetTint( const Rgba8& tint )												{ m_tint = tint; }

	UIPanel*	AddChildPanel( const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture = nullptr );
	UIButton*	AddButton( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* backgroundTexture = nullptr );

	AABB2 GetBoundingBox() const													{ return m_boundingBox; }

private:
	bool m_isActive = true;
	bool m_isVisible = true;

	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;

	// UI panel owns its child panels and buttons
	std::vector<UIPanel*> m_childPanels;
	std::vector<UIButton*> m_buttons;

	Texture* m_backgroundTexture = nullptr;
};

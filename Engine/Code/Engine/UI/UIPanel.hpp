#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/UI/UIElement.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class SpriteDefinition;
class Texture;
class UIButton;
class UILabel;


//-----------------------------------------------------------------------------------------------
class UIPanel : public UIElement
{
public:
	UIPanel( const UISystem& uiSystem, const AABB2& absoluteScreenBounds, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	UIPanel( const UISystem& uiSystem, UIPanel* parentPanel, const Vec2& widthFractionRange, const Vec2& heightFractionRange, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	~UIPanel();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;
	
	UIPanel*	AddChildPanel( const Vec2& widthFractionRange, const Vec2& heightFractionRange, 
							   Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	UIButton*	AddButton( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, 
						   Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );

	void		ClearButtons();

private:
	// UI panel owns its child panels and buttons
	std::vector<UIPanel*> m_childPanels;
	std::vector<UIButton*> m_buttons;
};

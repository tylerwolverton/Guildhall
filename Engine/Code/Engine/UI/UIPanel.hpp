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
	UIPanel( const UISystem& uiSystem, UIPanel* parentPanel, const UIAlignedPositionData& positionData, 
			 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
			 const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );

	UIPanel( const UISystem& uiSystem, UIPanel* parentPanel, const UIRelativePositionData& positionData, 
			 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
			 const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	~UIPanel();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	UIPanel*	AddChildPanel( const UIAlignedPositionData& positionData,
							   Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
							   const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	UIPanel*	AddChildPanel( const UIRelativePositionData& positionData,
							   Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
							   const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );

	UIButton*	AddButton( const UIAlignedPositionData& positionData,
							   Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	UIButton*	AddButton( const UIRelativePositionData& positionData,
							   Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );

	void		ClearButtons();

private:
	// UI panel owns its child panels and buttons
	std::vector<UIPanel*> m_childPanels;
	std::vector<UIButton*> m_buttons;
};
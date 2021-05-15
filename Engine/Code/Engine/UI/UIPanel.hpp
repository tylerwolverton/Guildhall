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
	friend class UISystem;

public:
	virtual ~UIPanel();

	virtual void Update() override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	virtual UIPanel* AddChildPanel( const UIAlignedPositionData& positionData,
									Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
									const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE ) override;

	virtual UIPanel* AddChildPanel( const UIRelativePositionData& positionData,
									Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
									const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE ) override;

	virtual UIPanel* AddChildPanel( const std::string& name,
									const UIAlignedPositionData& positionData,
									Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
									const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE ) override;

	virtual UIPanel* AddChildPanel( const std::string& name,
									const UIRelativePositionData& positionData,
									Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
									const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE ) override;


	virtual UIButton* AddButton( const UIAlignedPositionData& positionData,
								 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE ) override;

	virtual UIButton* AddButton( const UIRelativePositionData& positionData,
								 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE ) override;

	virtual UIButton* AddButton( const std::string& name,
								 const UIAlignedPositionData& positionData,
								 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE ) override;

	virtual UIButton* AddButton( const std::string& name,
								 const UIRelativePositionData& positionData,
								 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE ) override;

	virtual UIUniformGrid* AddUniformGrid( const std::string& name, const UIAlignedPositionData& gridPositionData,
										   const IntVec2& gridDimensions, const Vec2& paddingOfGridElementsPixels = Vec2::ZERO,
										   Texture* elementTexture = nullptr, const Rgba8& elementTint = Rgba8::WHITE ) override;

	void		ClearButtons();

private:
	UIPanel( UISystem& uiSystem, const AABB2& absoluteScreenBounds, Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE, const std::string& name = "" );
	UIPanel( UISystem& uiSystem, UIPanel* parentPanel, const UIAlignedPositionData& positionData,
			 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
			 const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE, 
			 const std::string& name = "" );

	UIPanel( UISystem& uiSystem, UIPanel* parentPanel, const UIRelativePositionData& positionData,
			 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
			 const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE,
			 const std::string& name = "" );

private:
	// UI panel owns its child panels and buttons
	std::vector<UIElement*> m_childPanels;
	std::vector<UIButton*> m_buttons;
};

#pragma once
#include "Engine/UI/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
class UIElement;
class SpriteDefinition;


//-----------------------------------------------------------------------------------------------
class UIImage : public UILabel
{
public:
	UIImage( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData, Texture* image = nullptr );
	UIImage( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData, Texture* image = nullptr );
	UIImage( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData, SpriteDefinition* spriteDef );
	UIImage( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData, SpriteDefinition* spriteDef );
	
	virtual void Render() const override;

private:
	Texture* m_image = nullptr;
	Vec2 m_uvAtMins = Vec2::ZERO;
	Vec2 m_uvAtMaxs = Vec2::ONE;
	
};

#pragma once
#include "Engine/UI/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
class UIElement;
class SpriteDefinition;


//-----------------------------------------------------------------------------------------------
class UIImage : public UILabel
{
	friend class UIElement;

public:	
	virtual void Render() const override;

private:
	UIImage( UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData, Texture* image = nullptr, const std::string& name = "" );
	UIImage( UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData, Texture* image = nullptr, const std::string& name = "" );
	UIImage( UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData, SpriteDefinition* spriteDef, const std::string& name = "" );
	UIImage( UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData, SpriteDefinition* spriteDef, const std::string& name = "" );

private:
	Texture* m_image = nullptr;
	Vec2 m_uvAtMins = Vec2::ZERO;
	Vec2 m_uvAtMaxs = Vec2::ONE;
	
};

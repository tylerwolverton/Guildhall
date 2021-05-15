#pragma once
#include "Game/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
class UIElement;
class SpriteDefinition;


//-----------------------------------------------------------------------------------------------
class UIImage : public UILabel
{
public:
	UIImage( const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* image = nullptr );
	UIImage( const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, SpriteDefinition* spriteDef );
	
	virtual void Render( RenderContext* renderer ) const override;

private:
	Texture* m_image;
	Vec2 m_uvAtMins = Vec2::ZERO;
	Vec2 m_uvAtMaxs = Vec2::ONE;
	
};

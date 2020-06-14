#pragma once
#include "Game/UI/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
class UIButton;
class SpriteDefinition;


//-----------------------------------------------------------------------------------------------
class UIImage : public UILabel
{
public:
	UIImage( const UIButton& parentButton, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* image = nullptr );
	UIImage( const UIButton& parentButton, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, SpriteDefinition* spriteDef );
	
	virtual void Render( RenderContext* renderer ) override;

private:
	Texture* m_image;
	Rgba8 m_tint = Rgba8::WHITE;
	Vec2 m_uvAtMins = Vec2::ZERO;
	Vec2 m_uvAtMaxs = Vec2::ONE;
	
};

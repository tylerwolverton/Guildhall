#pragma once
#include "Game/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
class UIButton;


//-----------------------------------------------------------------------------------------------
class UIImage : public UILabel
{
public:
	UIImage( const UIButton& parentButton, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* image = nullptr );

	virtual void Render( RenderContext* renderer ) override;

private:
	Texture* m_image;
	Rgba8 m_tint = Rgba8::WHITE;
};
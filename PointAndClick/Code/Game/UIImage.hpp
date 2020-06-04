#pragma once
#include "Game/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"

class UIButton;

class UIImage : public UILabel
{
public:
	UIImage( const UIButton& parentButton, const Texture& image );

	virtual void Render( Renderer* renderer ) override;

private:
	Texture m_image;
};

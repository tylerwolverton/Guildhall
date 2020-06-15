#pragma once
#include "Game/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
class UIButton;


//-----------------------------------------------------------------------------------------------
class UIText : public UILabel
{
public:
	UIText( const UIButton& parentButton, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, const std::string& text, float fontSize = 24.f );

	virtual void Render( RenderContext* renderer ) override;

private:
	std::string m_text;
	float m_fontSize = 24.f;
	Rgba8 m_tint = Rgba8::WHITE;
};

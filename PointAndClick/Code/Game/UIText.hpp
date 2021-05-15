#pragma once
#include "Game/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
class UIElement;


//-----------------------------------------------------------------------------------------------
class UIText : public UILabel
{
public:
	UIText( const UIElement& parentElement, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, const std::string& text, float fontSize = 24.f, const Vec2& alignment = ALIGN_CENTERED );

	virtual void Render( RenderContext* renderer ) const override;

	void SetText( const std::string& text )									{ m_text = text; }

private:
	std::string m_text;
	float m_fontSize = 24.f;
	Vec2 m_alignment = ALIGN_CENTERED;
};

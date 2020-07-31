#pragma once
#include "Engine/UI/UILabel.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
class UIElement;


//-----------------------------------------------------------------------------------------------
class UIText : public UILabel
{
public:
	UIText( const UISystem& uiSystem, const UIElement& parentElement, const UIAlignedPositionData& positionData, const std::string& text, float fontSize = 24.f, const Vec2& textAlignment = ALIGN_CENTERED );
	UIText( const UISystem& uiSystem, const UIElement& parentElement, const UIRelativePositionData& positionData, const std::string& text, float fontSize = 24.f, const Vec2& textAlignment = ALIGN_CENTERED );

	virtual void Render() const override;

	void SetText( const std::string& text )									{ m_text = text; }

private:
	std::string m_text;
	float m_fontSize = 24.f;
	Vec2 m_textAlignment = ALIGN_CENTERED;
};

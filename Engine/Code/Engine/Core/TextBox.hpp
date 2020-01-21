#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class RenderContext;
class Camera;


//-----------------------------------------------------------------------------------------------
struct TextBoxMessageLine
{
public:
	Rgba8 m_color = Rgba8::WHITE;
	std::string m_message;

public:
	explicit TextBoxMessageLine( const Rgba8& color, std::string message )
		: m_color( color )
		, m_message( message )
	{}
};


//-----------------------------------------------------------------------------------------------
class TextBox
{
public:
	explicit TextBox( RenderContext& renderer, const AABB2& localBoxBounds );
	~TextBox();

	void SetText( const Rgba8& textColor, const std::string& textBoxPrintString );
	void AddLineOFText( const Rgba8& textColor, const std::string& textBoxPrintString );
	void Render( const Vec2& worldMins ) const;
	
private:
	void RenderBackground( const Vec2& worldMins ) const;
	void RenderText( const Vec2& worldMins ) const;

private:
	RenderContext& m_renderer;
	AABB2 m_localBoxBounds = AABB2::ONE_BY_ONE;
	Rgba8 m_backgroundColor = Rgba8::BLACK;
	std::vector<TextBoxMessageLine> m_textBoxMessageLines;
};

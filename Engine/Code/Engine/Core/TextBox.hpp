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
	std::string m_message;
	Rgba8 m_color = Rgba8::WHITE;

public:
	explicit TextBoxMessageLine( std::string message, const Rgba8& color)
		: m_message( message )
		, m_color( color )
	{}
};


//-----------------------------------------------------------------------------------------------
class TextBox
{
public:
	explicit TextBox( RenderContext& renderer, const AABB2& localBoxBounds );
	~TextBox();

	void SetText( const std::string& textBoxPrintString, const Rgba8& textColor = Rgba8::WHITE );
	void AddLineOFText( const std::string& textBoxPrintString, const Rgba8& textColor = Rgba8::WHITE );
	void Render( const Vec2& worldMins ) const;
	
private:
	void RenderBackground( const Vec2& worldMins ) const;
	void RenderText( const Vec2& worldMins ) const;

	AABB2 GetTextBounds() const;

private:
	RenderContext& m_renderer;
	AABB2 m_localBoxBounds = AABB2::ONE_BY_ONE;
	Rgba8 m_backgroundColor = Rgba8::BLACK;
	std::vector<TextBoxMessageLine> m_textBoxMessageLines;
};

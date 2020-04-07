#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"


//-----------------------------------------------------------------------------------------------
TextBox::TextBox( RenderContext& renderer, const AABB2& localBoxBounds )
	: m_renderer( renderer )
	, m_localBoxBounds( localBoxBounds )
{
	m_backgroundColor.a = 150;
}


//-----------------------------------------------------------------------------------------------
TextBox::~TextBox()
{
}


//-----------------------------------------------------------------------------------------------
void TextBox::SetText( const std::string& textBoxPrintString, const Rgba8& textColor )
{
	m_textBoxMessageLines.clear();
	m_textBoxMessageLines.push_back( TextBoxMessageLine( textBoxPrintString, textColor ) );
}


//-----------------------------------------------------------------------------------------------
void TextBox::AddLineOFText( const std::string& textBoxPrintString, const Rgba8& textColor )
{
	m_textBoxMessageLines.push_back( TextBoxMessageLine( textBoxPrintString, textColor ) );
}


//-----------------------------------------------------------------------------------------------
void TextBox::Render( const Vec2& worldMins ) const
{
	RenderBackground( worldMins );
	RenderText( worldMins );
}


//-----------------------------------------------------------------------------------------------
void TextBox::RenderBackground( const Vec2& worldMins ) const
{
	std::vector<Vertex_PCU> backgroundVertices;

	AABB2 textBounds = GetTextBounds();

	AABB2 worldBoxBounds( worldMins + textBounds.mins, worldMins + textBounds.maxs );
	AppendVertsForAABB2D( backgroundVertices, worldBoxBounds, m_backgroundColor );

	m_renderer.BindDiffuseTexture( nullptr );
	m_renderer.DrawVertexArray( backgroundVertices );
}


//-----------------------------------------------------------------------------------------------
void TextBox::RenderText( const Vec2& worldMins ) const
{
	// Nothing to print
	if ( (int)m_textBoxMessageLines.size() == 0 )
	{
		return;
	}

	float boundsHeightInPixels = m_localBoxBounds.maxs.y - m_localBoxBounds.mins.y;
	float boundsWidthInPixels = m_localBoxBounds.maxs.x - m_localBoxBounds.mins.x;

	float lineHeight = boundsHeightInPixels / (float)m_textBoxMessageLines.size();
	float longestLineWidth = -1.f;
	for ( int textBoxMessageIndex = 0; textBoxMessageIndex < (int)m_textBoxMessageLines.size(); ++textBoxMessageIndex )
	{
		float curLineWidth = lineHeight * m_textBoxMessageLines[textBoxMessageIndex].m_message.length();
		if ( curLineWidth > longestLineWidth )
		{
			longestLineWidth = curLineWidth;
		}
	}

	if ( longestLineWidth > boundsWidthInPixels )
	{
		lineHeight *= boundsWidthInPixels / longestLineWidth;
	}

	// TODO: Use triangle font if font not available
	BitmapFont* font = m_renderer.GetSystemFont();

	std::vector<Vertex_PCU> vertices;
	float curLineY = 0;
	int latestMessageIndex = (int)m_textBoxMessageLines.size() - 1;

	for ( int logMessageIndexFromEnd = 0; logMessageIndexFromEnd < (int)m_textBoxMessageLines.size(); ++logMessageIndexFromEnd )
	{
		int logMessageIndex = latestMessageIndex - logMessageIndexFromEnd;
		const TextBoxMessageLine& logMessage = m_textBoxMessageLines[logMessageIndex];

		if ( font->GetTexture() != nullptr )
		{
			font->AppendVertsForText2D( vertices, Vec2( worldMins.x + m_localBoxBounds.mins.x, worldMins.y + m_localBoxBounds.mins.y + curLineY ),
										lineHeight, logMessage.m_message, logMessage.m_color, .7f );
		}
		else
		{
			AppendTextTriangles2D( vertices, logMessage.m_message, Vec2( worldMins.x + m_localBoxBounds.mins.x, worldMins.y + m_localBoxBounds.mins.y + curLineY ),
								   lineHeight, logMessage.m_color );
		}
		curLineY += lineHeight;
	}

	if ( font->GetTexture() != nullptr )
	{
		m_renderer.BindDiffuseTexture( font->GetTexture() );
	}
	else
	{
		m_renderer.BindDiffuseTexture( nullptr );
	}
	m_renderer.DrawVertexArray( vertices );
}

//-----------------------------------------------------------------------------------------------
AABB2 TextBox::GetTextBounds() const
{
	float boundsHeightInPixels = m_localBoxBounds.maxs.y - m_localBoxBounds.mins.y;
	float boundsWidthInPixels = m_localBoxBounds.maxs.x - m_localBoxBounds.mins.x;

	float lineHeight = boundsHeightInPixels / (float)m_textBoxMessageLines.size();
	float longestLineWidth = -1.f;
	for ( int textBoxMessageIndex = 0; textBoxMessageIndex < (int)m_textBoxMessageLines.size(); ++textBoxMessageIndex )
	{
		float curLineWidth = lineHeight * m_textBoxMessageLines[textBoxMessageIndex].m_message.length();
		if ( curLineWidth > longestLineWidth )
		{
			longestLineWidth = curLineWidth;
		}
	}

	float maxWidth = longestLineWidth;
	if ( longestLineWidth > boundsWidthInPixels )
	{
		lineHeight *= boundsWidthInPixels / longestLineWidth;
		maxWidth = boundsWidthInPixels;
	}

	return AABB2( m_localBoxBounds.mins, Vec2( maxWidth, lineHeight * (int)m_textBoxMessageLines.size() ) );
}

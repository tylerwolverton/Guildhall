#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"

//-----------------------------------------------------------------------------------------------
BitmapFont::BitmapFont( const char* fontName, const Texture* fontTexture )
	: m_fontName( fontName )
	, m_glyphSpriteSheet( *fontTexture, IntVec2( 16, 16 ) )
{
}


//-----------------------------------------------------------------------------------------------
const Texture* BitmapFont::GetTexture() const
{
	return &( m_glyphSpriteSheet.GetTexture() );
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::AppendVertsForText2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect )
{
	float cellWidth = cellHeight * cellAspect;

	for( int charIndex = 0; charIndex < text.length(); ++charIndex )
	{
		Vec2 charMins( textMins.x + ( charIndex * cellWidth ), textMins.y );
		Vec2 charMaxs( charMins.x + cellWidth, charMins.y + cellHeight );

		Vec2 uvAtMins, uvAtMaxs;
		m_glyphSpriteSheet.GetSpriteUVs( uvAtMins, uvAtMaxs, text[charIndex] );

		vertexArray.push_back( Vertex_PCU( charMins,						tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( Vec2( charMaxs.x, charMins.y ),	tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
		vertexArray.push_back( Vertex_PCU( charMaxs,						tint, uvAtMaxs ) );

		vertexArray.push_back( Vertex_PCU( charMins,						tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( charMaxs,						tint, uvAtMaxs ) );
		vertexArray.push_back( Vertex_PCU( Vec2( charMins.x, charMaxs.y ),	tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	}
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::AppendVertsAndIndicesForText2D( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect )
{
	float cellWidth = cellHeight * cellAspect;
	int indexNum = 0;
	std::vector<uint> indices;
	for ( int charIndex = 0; charIndex < text.length(); ++charIndex )
	{
		Vec2 charMins( textMins.x + ( charIndex * cellWidth ), textMins.y );
		Vec2 charMaxs( charMins.x + cellWidth, charMins.y + cellHeight );

		Vec2 uvAtMins, uvAtMaxs;
		m_glyphSpriteSheet.GetSpriteUVs( uvAtMins, uvAtMaxs, text[charIndex] );

		vertexArray.push_back( Vertex_PCU( charMins, tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( Vec2( charMaxs.x, charMins.y ), tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
		vertexArray.push_back( Vertex_PCU( Vec2( charMins.x, charMaxs.y ), tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
		vertexArray.push_back( Vertex_PCU( charMaxs, tint, uvAtMaxs ) );

		indices.push_back( indexNum );
		indices.push_back( indexNum + 1);
		indices.push_back( indexNum + 3);
		indices.push_back( indexNum );
		indices.push_back( indexNum + 3);
		indices.push_back( indexNum + 2);

		indexNum += 4;
	}

	indexArray.insert( indexArray.end(), indices.begin(), indices.end() );

	// Add reversed indices to show back of text
	std::vector<uint> reversedIndices;
	for ( int indicesIdx = (int)indices.size() - 1; indicesIdx >= 0; --indicesIdx )
	{
		reversedIndices.push_back( indices[indicesIdx] );
	}

	indexArray.insert( indexArray.end(), reversedIndices.begin(), reversedIndices.end() );
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::AppendVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect, const Vec2& alignment )
{
	Vec2 textDimensions( GetDimensionsForText2D( cellHeight, text, cellAspect ) );
	Vec2 boxDimensions( box.maxs - box.mins );

	Vec2 differenceInSize( boxDimensions - textDimensions );

	Vec2 textMins = box.mins + ( differenceInSize * alignment );

	AppendVertsForText2D( vertexArray, textMins, cellHeight, text, tint, cellAspect );
}


//-----------------------------------------------------------------------------------------------
Vec2 BitmapFont::GetDimensionsForText2D( float cellHeight, const std::string& text, float cellAspect )
{
	float cellWidth = cellHeight * cellAspect;

	return Vec2( cellWidth * text.size(), cellHeight );
}


//-----------------------------------------------------------------------------------------------
float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	UNUSED( glyphUnicode );

	return 1.f;
}

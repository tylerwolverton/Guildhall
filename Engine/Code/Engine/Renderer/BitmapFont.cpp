#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexFont.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
BitmapFont::BitmapFont( const std::string& fontName, const Texture* fontTexture )
	: m_fontName( fontName )
{
	m_glyphSpriteSheet = new SpriteSheet( *fontTexture, IntVec2( 16, 16 ) );
}


//-----------------------------------------------------------------------------------------------
BitmapFont::BitmapFont( const std::string& fontName, const Texture* fontTexture, const std::string& metaDataFilePath )
	: m_fontName( fontName )
{
	XmlDocument doc;
	XmlError loadError = doc.LoadFile( metaDataFilePath.c_str() );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( Stringf( "Font metadata: %s could not be opened", metaDataFilePath .c_str() ) );
		return;
	}

	XmlElement* root = doc.RootElement();

	//XmlElement* infoElem = root->FirstChildElement( "info" );
	//<common lineHeight = "128" base = "100" scaleW = "2048" scaleH = "2048" pages = "1" packed = "0" alphaChnl = "0" redChnl = "4" greenChnl = "4" blueChnl = "4"/>

	XmlElement* commonElem = root->FirstChildElement( "common" );
	if ( commonElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Font metadata: %s missing common element", metaDataFilePath.c_str() ) );
		return;
	}

	float lineHeight = ParseXmlAttribute( *commonElem, "lineHeight", 16.f );
	if( IsNearlyEqual( lineHeight, 0.f ) )
	{
		g_devConsole->PrintError( Stringf( "Font %s has a lineHeight of 0", fontName.c_str() ) );
		return;
	}

	int scaleWidth = ParseXmlAttribute( *commonElem, "scaleW", 16 );
	int scaleHeight = ParseXmlAttribute( *commonElem, "scaleH", 16 );
	if ( scaleWidth == 0 || scaleHeight == 0 )
	{
		g_devConsole->PrintError( Stringf( "Font %s has a scaleW or scaleH of 0", fontName.c_str() ) );
		return;
	}

	XmlElement* charsElem = root->FirstChildElement( "chars" );
	if ( charsElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Font metadata: %s missing chars element", metaDataFilePath.c_str() ) );
		return;
	}

	std::vector<AABB2> uvBoundsPerSprite;

	//int numChars = ParseXmlAttribute( *charsElem, "count", 0 );
	int charsProcessed = 0;
	XmlElement* charElem = charsElem->FirstChildElement( "char" );
	while ( charElem != nullptr )
	{
		// Read in char data
		float xPixels = ParseXmlAttribute( *charElem, "x", 0.f );
		float yPixels = ParseXmlAttribute( *charElem, "y", 0.f );
		float widthPixels = ParseXmlAttribute( *charElem, "width", 1.f );
		float heightPixels = ParseXmlAttribute( *charElem, "height", 1.f );

		float xOffsetPixels = ParseXmlAttribute( *charElem, "xoffset", 0.f );
		float yOffsetPixels = ParseXmlAttribute( *charElem, "yoffset", 0.f );
		float xAdvancePixels = ParseXmlAttribute( *charElem, "xadvance", 0.f );
		float postAdvancePixels = xAdvancePixels - widthPixels;


		// Normalize pixel values by the total scale for uvs
		float xNormalized = xPixels / (float)scaleWidth;
		float yNormalized = yPixels / (float)scaleHeight;
		yNormalized = 1.f - yNormalized;
		float widthNormalized = widthPixels / (float)scaleWidth;
		float heightNormalized = heightPixels / (float)scaleHeight;

		uvBoundsPerSprite.emplace_back( xNormalized, yNormalized - heightNormalized, xNormalized + widthNormalized, yNormalized );

		// Normalize pixel values by lineHeight for metadata
		//postAdvancePixels /= ( glyphAspect * lineHeight );
		float glyphAspect = widthPixels / lineHeight;
		float xOffsetNormalized = xOffsetPixels / lineHeight;
		float postAdvanceNormalized = postAdvancePixels / lineHeight;
		float yOffsetNormalized = yOffsetPixels / lineHeight;

		// Save glyph metadata
		GlyphData glyphData;
		glyphData.spriteIdx = charsProcessed;
		
		glyphData.preAdvance = xOffsetNormalized;
		glyphData.glyphAspect = glyphAspect;
		glyphData.postAdvance = postAdvanceNormalized - xOffsetNormalized;

		glyphData.yOffsetNormalized = yOffsetNormalized;
		glyphData.heightNormalized = heightPixels / lineHeight;

		int charId = ParseXmlAttribute( *charElem, "id", 0 );
		m_glyphMetaData[charId] = glyphData;

		// Advance to next char
		charElem = charElem->NextSiblingElement( "char" );
		++charsProcessed;
	}

	m_glyphSpriteSheet = new SpriteSheet( *fontTexture, uvBoundsPerSprite );
}


//-----------------------------------------------------------------------------------------------
BitmapFont::~BitmapFont()
{
	PTR_SAFE_DELETE( m_glyphSpriteSheet );
}


//-----------------------------------------------------------------------------------------------
const Texture* BitmapFont::GetTexture() const
{
	if ( m_glyphSpriteSheet == nullptr )
	{
		return nullptr;
	}

	return &( m_glyphSpriteSheet->GetTexture() );
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::AppendVertsForText2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect )
{
	if ( m_glyphSpriteSheet == nullptr )
	{
		return;
	}

	// Check if we need to use metadata instead
	if ( !m_glyphMetaData.empty() )
	{
		AppendVertsForText2DWithMetadata( vertexArray, textMins, cellHeight, text, tint );
		return;
	}

	float cellWidth = cellHeight * cellAspect;
	
	vertexArray.reserve( vertexArray.size() + text.length() * 6 );

	for( int charIndex = 0; charIndex < text.length(); ++charIndex )
	{
		Vec2 charMins( textMins.x + ( charIndex * cellWidth ), textMins.y );
		Vec2 charMaxs( charMins.x + cellWidth, charMins.y + cellHeight );

		Vec2 uvAtMins, uvAtMaxs;
		m_glyphSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, text[charIndex] );

		vertexArray.push_back( Vertex_PCU( charMins,						tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( Vec2( charMaxs.x, charMins.y ),	tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
		vertexArray.push_back( Vertex_PCU( charMaxs,						tint, uvAtMaxs ) );

		vertexArray.push_back( Vertex_PCU( charMins,						tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( charMaxs,						tint, uvAtMaxs ) );
		vertexArray.push_back( Vertex_PCU( Vec2( charMins.x, charMaxs.y ),	tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	}
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::AppendVertsForText2D( std::vector<VertexFont>& vertexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint, const Vec4& specialEffects )
{
	vertexArray.reserve( vertexArray.size() + text.length() * 6 );

	float lineWidth = GetDimensionsForProportionalText2D( cellHeight, text ).x;
	Vec2 charMins = textMins;
	for ( int charIndex = 0; charIndex < text.length(); ++charIndex )
	{
		GlyphData glyphData = m_glyphMetaData[text[charIndex]];
		charMins.x += glyphData.preAdvance * cellHeight;

		float charWidth = glyphData.glyphAspect * cellHeight;
		float charHeight = glyphData.heightNormalized * cellHeight;

		Vec2 charMaxs( charMins.x + charWidth, charMins.y + charHeight );

		// Account for the yoffset for this char. The offset is given as space to move down from top of the letter
		charMaxs.y = ( textMins.y + cellHeight ) - ( glyphData.yOffsetNormalized * cellHeight );
		charMins.y = charMaxs.y - charHeight;

		// Add vertexes with correct uvs
		Vec2 uvAtMins, uvAtMaxs;
		m_glyphSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, glyphData.spriteIdx );

		Vec2 textPosMins( charMins.x / lineWidth, 0.f );
		Vec2 textPosMaxs( charMaxs.x / lineWidth, cellHeight );

		vertexArray.push_back( VertexFont( charMins,						tint, uvAtMins,							Vec2::ZERO,		textPosMins,							 charIndex, specialEffects ) );
		vertexArray.push_back( VertexFont( Vec2( charMaxs.x, charMins.y ),	tint, Vec2( uvAtMaxs.x, uvAtMins.y ),	Vec2::ONE_ZERO, Vec2( textPosMaxs.x, textPosMins.y ),	 charIndex, specialEffects ) );
		vertexArray.push_back( VertexFont( charMaxs,						tint, uvAtMaxs,							Vec2::ONE,		textPosMaxs,							 charIndex, specialEffects ) );
																																											 
		vertexArray.push_back( VertexFont( charMins,						tint, uvAtMins,							Vec2::ZERO,		textPosMins,							 charIndex, specialEffects ) );
		vertexArray.push_back( VertexFont( charMaxs,						tint, uvAtMaxs,							Vec2::ONE,		textPosMaxs,							 charIndex, specialEffects ) );
		vertexArray.push_back( VertexFont( Vec2( charMins.x, charMaxs.y ),	tint, Vec2( uvAtMins.x, uvAtMaxs.y ),	Vec2::ZERO_TO_ONE, Vec2( textPosMins.x, textPosMaxs.y ), charIndex, specialEffects ) );

		// Advance to next char start pos
		charMins.x = charMaxs.x + glyphData.postAdvance * cellHeight;
		charMins.y = textMins.y;
	}
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::AppendVertsAndIndicesForText2D( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect )
{
	if ( m_glyphSpriteSheet == nullptr )
	{
		return;
	}

	float cellWidth = cellHeight * cellAspect;
	int indexNum = (int)vertexArray.size();
	std::vector<uint> indices;
	for ( int charIndex = 0; charIndex < text.length(); ++charIndex )
	{
		Vec2 charMins( textMins.x + ( charIndex * cellWidth ), textMins.y );
		Vec2 charMaxs( charMins.x + cellWidth, charMins.y + cellHeight );

		Vec2 uvAtMins, uvAtMaxs;
		m_glyphSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, text[charIndex] );

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
Vec2 BitmapFont::GetDimensionsForProportionalText2D( float cellHeight, const std::string& text )
{
	Vec2 dimensions = Vec2( 0.f, cellHeight );
	for ( int charIndex = 0; charIndex < text.length(); ++charIndex )
	{
		GlyphData glyphData = m_glyphMetaData[text[charIndex]];
		dimensions.x += glyphData.preAdvance * cellHeight;
		dimensions.x += glyphData.glyphAspect * cellHeight;
		dimensions.x += glyphData.postAdvance * cellHeight;
	}

	return dimensions;
}


//-----------------------------------------------------------------------------------------------
float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	UNUSED( glyphUnicode );

	return 1.f;
}


//-----------------------------------------------------------------------------------------------
void BitmapFont::AppendVertsForText2DWithMetadata( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint )
{
	vertexArray.reserve( vertexArray.size() + text.length() * 6 );

	Vec2 charMins = textMins;
	for ( int charIndex = 0; charIndex < text.length(); ++charIndex )
	{
		GlyphData glyphData = m_glyphMetaData[text[charIndex]];
		charMins.x += glyphData.preAdvance * cellHeight;
		
		float charWidth = glyphData.glyphAspect * cellHeight;
		float charHeight = glyphData.heightNormalized * cellHeight;

		Vec2 charMaxs( charMins.x + charWidth, charMins.y + charHeight );

		// Account for the yoffset for this char. The offset is given as space to move down from top of the letter
		charMaxs.y = ( textMins.y + cellHeight ) - ( glyphData.yOffsetNormalized * cellHeight );
		charMins.y = charMaxs.y - charHeight;
		
		// Add vertexes with correct uvs
		Vec2 uvAtMins, uvAtMaxs;
		m_glyphSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, glyphData.spriteIdx );

		vertexArray.push_back( Vertex_PCU( charMins, tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( Vec2( charMaxs.x, charMins.y ), tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
		vertexArray.push_back( Vertex_PCU( charMaxs, tint, uvAtMaxs ) );

		vertexArray.push_back( Vertex_PCU( charMins, tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( charMaxs, tint, uvAtMaxs ) );
		vertexArray.push_back( Vertex_PCU( Vec2( charMins.x, charMaxs.y ), tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );

		// Advance to next char start pos
		charMins.x = charMaxs.x + glyphData.postAdvance * cellHeight;
		charMins.y = textMins.y;
	}
}

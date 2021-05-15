#include "VertexFont.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
// Static variables
const BufferAttribute VertexFont::LAYOUT[] =
{
   BufferAttribute( "POSITION",			BUFFER_FORMAT_VEC3,      		offsetof( VertexFont, position ) ),
   BufferAttribute( "COLOR",			BUFFER_FORMAT_R8G8B8A8_UNORM, 	offsetof( VertexFont, color ) ),
   BufferAttribute( "TEXCOORD",			BUFFER_FORMAT_VEC2,				offsetof( VertexFont, uvTexCoords ) ),
   BufferAttribute( "GLYPHPOSITION",	BUFFER_FORMAT_VEC2,				offsetof( VertexFont, glyphPosition ) ),
   BufferAttribute( "TEXTPOSITION",		BUFFER_FORMAT_VEC2,				offsetof( VertexFont, textPosition ) ),
   BufferAttribute( "CHARINDEX",		BUFFER_FORMAT_INT,				offsetof( VertexFont, characterIndex ) ),
   BufferAttribute( "SPECIALEFFECTS",	BUFFER_FORMAT_VEC4,				offsetof( VertexFont, specialEffects ) ),
   BufferAttribute() // end - terminator element; 
};


//-----------------------------------------------------------------------------------------------
VertexFont::VertexFont( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec2& glyphPosition, const Vec2& textPosition, int characterIndex, const Vec4& specialEffects )
	: position( position )
	, color( tint )
	, uvTexCoords( uvTexCoords )
	, glyphPosition( glyphPosition )
	, textPosition( textPosition )
	, characterIndex( characterIndex )
	, specialEffects( specialEffects )
{
}


//-----------------------------------------------------------------------------------------------
VertexFont::VertexFont( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec2& glyphPosition, const Vec2& textPosition, int characterIndex )
	: position( position )
	, color( tint )
	, uvTexCoords( uvTexCoords )
	, glyphPosition( glyphPosition )
	, textPosition( textPosition )
	, characterIndex( characterIndex )
{
}


//-----------------------------------------------------------------------------------------------
VertexFont::VertexFont( const Vec2& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec2& glyphPosition, const Vec2& textPosition, int characterIndex )
	: position( Vec3( position.x, position.y, 0.f ) )
	, color( tint )
	, uvTexCoords( uvTexCoords )
	, glyphPosition( glyphPosition )
	, textPosition( textPosition )
	, characterIndex( characterIndex )
{
}


//-----------------------------------------------------------------------------------------------
VertexFont::VertexFont( const Vec2& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec2& glyphPosition, const Vec2& textPosition, int characterIndex, const Vec4& specialEffects )
	: position( Vec3( position.x, position.y, 0.f ) )
	, color( tint )
	, uvTexCoords( uvTexCoords )
	, glyphPosition( glyphPosition )
	, textPosition( textPosition )
	, characterIndex( characterIndex )
	, specialEffects( specialEffects )
{
}



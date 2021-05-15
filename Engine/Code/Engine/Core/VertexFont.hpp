#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/BufferAttribute.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct VertexFont
{
public:
	Vec3  position;
	Rgba8 color;
	Vec2  uvTexCoords;

	Vec2 glyphPosition;		// normalized [0,1] coordinates of where this vert lies within this glyph-quad
	Vec2 textPosition;		// normalized coordinates of where this vert is within this overall text block
	int characterIndex = 0; // index of which character in this text-string this vert belongs to( same at all four corners of each glyph-quad )

	Vec4 specialEffects;

public:
	// Construction
	VertexFont() {} // Default constructor (do nothing, all member variables will default construct themselves)
	explicit VertexFont( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec2& glyphPosition, const Vec2& textPosition, int characterIndex, const Vec4& specialEffects );
	explicit VertexFont( const Vec2& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec2& glyphPosition, const Vec2& textPosition, int characterIndex, const Vec4& specialEffects );
	explicit VertexFont( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec2& glyphPosition, const Vec2& textPosition, int characterIndex );
	explicit VertexFont( const Vec2& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec2& glyphPosition, const Vec2& textPosition, int characterIndex );

	static const BufferAttribute LAYOUT[];
};

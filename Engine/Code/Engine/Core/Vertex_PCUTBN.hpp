#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/BufferAttribute.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vertex_PCUTBN
{
public:
	Vec3  position;
	Rgba8 color;
	Vec2  uvTexCoords;

	Vec3 tangent;
	Vec3 bitangent;
	Vec3 normal;

public:
	// Construction
	Vertex_PCUTBN() {} // Default constructor (do nothing, all member variables will default construct themselves)
	explicit Vertex_PCUTBN( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords, const Vec3& normal, const Vec3& tangent );

	bool operator==( const Vertex_PCUTBN& other );

	static const BufferAttribute LAYOUT[];
};

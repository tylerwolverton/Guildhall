#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/BufferAttribute.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vertex_PCU
{
public:
	Vec3  m_position;
	Rgba8 m_color;
	Vec2  m_uvTexCoords;

public:
	// Construction
	Vertex_PCU() {} // Default constructor (do nothing, all member variables will default construct themselves)
	explicit Vertex_PCU( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords );
	explicit Vertex_PCU( const Vec2& position, const Rgba8& tint, const Vec2& uvTexCoords );
	explicit Vertex_PCU( const Vec2& position, const Rgba8& tint );

	static const BufferAttribute LAYOUT[];

	static void TransformVertexArray( Vertex_PCU* vertexArray, int vertexCount, float uniformScale, float orientationDegrees, const Vec2& translation );
	static void TransformVertexArray( std::vector<Vertex_PCU>& vertices, float uniformScale, float orientationDegrees, const Vec2& translation );
	static void TransformVertexArray( Vertex_PCU* vertexArray, int vertexCount, float uniformScale, float orientationDegrees, const Vec3& translation );
	static void TransformVertexArray( std::vector<Vertex_PCU>& vertices, float uniformScale, float orientationDegrees, const Vec3& translation );
};

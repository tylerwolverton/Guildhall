#include "Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Vertex_PCU::Vertex_PCU( const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords )
	: m_position(position)
	, m_color(tint)
	, m_uvTexCoords(uvTexCoords)
{
}


//-----------------------------------------------------------------------------------------------
Vertex_PCU::Vertex_PCU( const Vec2& position, const Rgba8& tint, const Vec2& uvTexCoords )
	: m_position( Vec3( position.x, position.y, 0.f ) )
	, m_color( tint )
	, m_uvTexCoords( uvTexCoords )
{
}


//-----------------------------------------------------------------------------------------------
Vertex_PCU::Vertex_PCU( const Vec2& position, const Rgba8& tint )
	: m_position( Vec3( position.x, position.y, 0.f ) )
	, m_color( tint )
	, m_uvTexCoords( Vec2( 0.f, 0.f ) )
{
}


//-----------------------------------------------------------------------------------------------
void Vertex_PCU::TransformVertexArray( Vertex_PCU* vertexArray, int vertexCount, float uniformScale, float orientationDegrees, const Vec2& translation )
{
	if ( vertexArray == nullptr )
	{
		return;
	}

	for ( int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex )
	{
		Vertex_PCU& vertex = vertexArray[vertexIndex];
		Vec2 vertexXY( vertex.m_position.x, vertex.m_position.y );
		
		Vec2 newPos = TransformPosition2D( vertexXY, uniformScale, orientationDegrees, translation );
		
		vertex.m_position.x = newPos.x;
		vertex.m_position.y = newPos.y;
	}
}


//-----------------------------------------------------------------------------------------------
void Vertex_PCU::TransformVertexArray( std::vector<Vertex_PCU>& vertices, float uniformScale, float orientationDegrees, const Vec2& translation )
{
	TransformVertexArray( &vertices[0], (int)vertices.size(), uniformScale, orientationDegrees, translation );
}

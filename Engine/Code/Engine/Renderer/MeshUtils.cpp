#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Polygon2.hpp"


//-----------------------------------------------------------------------------------------------
void AppendVertsForArc( std::vector<Vertex_PCU>& vertexArray, const Vec2& center, float radius, float arcAngleDegrees, float startOrientationDegrees, const Rgba8& tint )
{
	constexpr float NUM_SIDES = 64.f;
	float degreesPerSide = arcAngleDegrees / NUM_SIDES;

	for ( int segmentNum = 0; segmentNum < NUM_SIDES; ++segmentNum )
	{
		float thetaDeg = startOrientationDegrees + ( degreesPerSide * (float)segmentNum );
		float theta2Deg = startOrientationDegrees + ( degreesPerSide * (float)( segmentNum + 1 ) );

		Vec2 start( radius * CosDegrees( thetaDeg ),
					radius * SinDegrees( thetaDeg ) );

		Vec2 end( radius * CosDegrees( theta2Deg ),
				  radius * SinDegrees( theta2Deg ) );

		// Add triangle segment
		vertexArray.push_back( Vertex_PCU( center, tint ) );
		vertexArray.push_back( Vertex_PCU( center + start, tint ) );
		vertexArray.push_back( Vertex_PCU( center + end, tint ) );
	}
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	vertexArray.push_back( Vertex_PCU( spriteBounds.mins, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( Vec2( spriteBounds.maxs.x, spriteBounds.mins.y ), tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( spriteBounds.maxs, tint, uvAtMaxs ) );

	vertexArray.push_back( Vertex_PCU( spriteBounds.mins, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( spriteBounds.maxs, tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( Vec2( spriteBounds.mins.x, spriteBounds.maxs.y ), tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const OBB2& spriteBounds, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec2 boxHalfWidth( spriteBounds.m_halfDimensions.x * spriteBounds.m_iBasis );
	Vec2 boxHalfHeight( spriteBounds.m_halfDimensions.y * spriteBounds.GetJBasisNormal() );

	Vec2 topRight( spriteBounds.m_center + boxHalfWidth + boxHalfHeight );
	Vec2 topLeft( spriteBounds.m_center - boxHalfWidth + boxHalfHeight );
	Vec2 bottomLeft( spriteBounds.m_center - boxHalfWidth - boxHalfHeight );
	Vec2 bottomRight( spriteBounds.m_center + boxHalfWidth - boxHalfHeight );

	AppendVertsForOBB2D( vertexArray, bottomLeft, bottomRight, topLeft, topRight, tint, uvAtMins, uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& bottomLeft, const Vec2& bottomRight, const Vec2& topLeft, const Vec2& topRight, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	vertexArray.push_back( Vertex_PCU( bottomLeft, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( bottomRight, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( topRight, tint, uvAtMaxs ) );

	vertexArray.push_back( Vertex_PCU( bottomLeft, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( topRight, tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( topLeft, tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, const Capsule2& capsule, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec2 iBasis = capsule.m_middleEnd - capsule.m_middleStart;
	iBasis.Normalize();
	iBasis.Rotate90Degrees();

	// Add middle box
	AppendVertsForOBB2D( vertexArray, capsule.GetCenterSectionAsOBB2(), tint, uvAtMins, uvAtMaxs );

	// Add end caps
	AppendVertsForArc( vertexArray, capsule.m_middleStart, capsule.m_radius, 180.f, iBasis.GetOrientationDegrees(), tint );
	AppendVertsForArc( vertexArray, capsule.m_middleEnd, capsule.m_radius, 180.f, iBasis.GetRotatedDegrees( -180.f ).GetOrientationDegrees(), tint );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForPolygon2( std::vector<Vertex_PCU>& vertexArray, const std::vector<Vec2>& vertexPositions, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	int numVertices = (int)vertexPositions.size();

	if ( numVertices < 3 )
	{
		g_devConsole->PrintString( Stringf( "Tried to append verts for a Polygon2 with %d vertices. At least 3 vertices are required.", numVertices ), Rgba8::YELLOW );
		return;
	}

	for ( int vertexPosIdx = 1; vertexPosIdx < numVertices - 1; ++vertexPosIdx )
	{
		int nextVertexIdx = vertexPosIdx + 1;
		vertexArray.push_back( Vertex_PCU( vertexPositions[0], tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( vertexPositions[vertexPosIdx], tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
		vertexArray.push_back( Vertex_PCU( vertexPositions[nextVertexIdx], tint, uvAtMaxs ) );
	}
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForCubeMesh( std::vector<Vertex_PCU>& vertexArray, const Vec3& center, float sideLength, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec3 mins( center );
	mins.x -= sideLength * .5f;
	mins.y -= sideLength * .5f;
	mins.z += sideLength * .5f;

	Vec3 maxs( center );
	maxs.x += sideLength * .5f;
	maxs.y += sideLength * .5f;
	maxs.z -= sideLength * .5f;

	// Front 4 points
	Vec3 vert0( mins );
	Vec3 vert1( maxs.x, mins.y, mins.z );
	Vec3 vert2( mins.x, maxs.y, mins.z );
	Vec3 vert3( maxs.x, maxs.y, mins.z );

	Vec3 backMins( mins );
	backMins.z = center.z - sideLength * .5f;

	Vec3 backMaxs( maxs );
	backMaxs.z = center.z + sideLength * .5f;

	// Back 4 points ( from front perspective for directions )	
	Vec3 vert4( backMins );
	Vec3 vert5( backMaxs.x, backMins.y, backMins.z );
	Vec3 vert6( backMins.x, backMaxs.y, backMins.z );
	Vec3 vert7( backMaxs.x, backMaxs.y, backMins.z );

	vertexArray.reserve( 24 );
	// Front
	vertexArray.push_back( Vertex_PCU( vert0, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( vert1, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert2, tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert3, tint, uvAtMaxs ) );

	// Right
	vertexArray.push_back( Vertex_PCU( vert1, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( vert5, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert3, tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert7, tint, uvAtMaxs ) );

	// Back
	vertexArray.push_back( Vertex_PCU( vert4, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert5, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( vert6, tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( vert7, tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );

	// Left
	vertexArray.push_back( Vertex_PCU( vert4, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( vert0, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert6, tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert2, tint, uvAtMaxs ) );

	// Top
	vertexArray.push_back( Vertex_PCU( vert2, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( vert3, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert6, tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert7, tint, uvAtMaxs ) );

	// Bottom
	vertexArray.push_back( Vertex_PCU( vert0, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( vert1, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( vert4, tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( vert5, tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}


//-----------------------------------------------------------------------------------------------
void AppendIndicesForCubeMesh( std::vector<uint>& indices )
{
	indices.reserve( 36 );
	// Front face
	indices.push_back( 0 );
	indices.push_back( 1 );
	indices.push_back( 3 );

	indices.push_back( 0 );
	indices.push_back( 3 );
	indices.push_back( 2 );

	// Right face
	indices.push_back( 4 );
	indices.push_back( 5 );
	indices.push_back( 7 );

	indices.push_back( 4 );
	indices.push_back( 7 );
	indices.push_back( 6 );

	// Back face
	indices.push_back( 9 );
	indices.push_back( 8 );
	indices.push_back( 10 );

	indices.push_back( 9 );
	indices.push_back( 10 );
	indices.push_back( 11 );

	// Left face
	indices.push_back( 12 );
	indices.push_back( 13 );
	indices.push_back( 15 );

	indices.push_back( 12 );
	indices.push_back( 15 );
	indices.push_back( 14 );

	// Top face
	indices.push_back( 16 );
	indices.push_back( 17 );
	indices.push_back( 19 );

	indices.push_back( 16 );
	indices.push_back( 19 );
	indices.push_back( 18 );

	// Bottom face
	indices.push_back( 20 );
	indices.push_back( 22 );
	indices.push_back( 23 );

	indices.push_back( 20 );
	indices.push_back( 23 );
	indices.push_back( 21 );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForAABB2DWithDepth( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds, float zDepth, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec3 mins( spriteBounds.mins, zDepth );
	Vec3 maxs( spriteBounds.maxs, zDepth );

	vertexArray.push_back( Vertex_PCU( mins, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( Vec3( spriteBounds.maxs.x, spriteBounds.mins.y, zDepth ), tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( maxs, tint, uvAtMaxs ) );

	vertexArray.push_back( Vertex_PCU( mins, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( maxs, tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( Vec3( spriteBounds.mins.x, spriteBounds.maxs.y, zDepth ), tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}

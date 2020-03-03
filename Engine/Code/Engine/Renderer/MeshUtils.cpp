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
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
void DrawLine2D( RenderContext* renderer, const Vec2& start, const Vec2& end, const Rgba8& color, float thickness )
{
	float radius = 0.5f * thickness;
	Vec2 displacement = end - start;

	// Create a small vector to be used to add a little
	// extra to the end of each line to make overlapping look better
	Vec2 forward = displacement.GetNormalized();
	forward *= radius;

	Vec2 left = forward.GetRotated90Degrees();

	// Calculate each corner of the box that will represent the line
	Vec2 startLeft = start - forward + left;
	Vec2 startRight = start - forward - left;
	Vec2 endLeft = end + forward + left;
	Vec2 endRight = end + forward - left;

	Vertex_PCU lineVertices[] =
	{
		Vertex_PCU( startRight, color ),
		Vertex_PCU( endRight, color ),
		Vertex_PCU( endLeft, color ),

		Vertex_PCU( startRight, color ),
		Vertex_PCU( endLeft, color ),
		Vertex_PCU( startLeft, color )
	};

	constexpr int NUM_VERTICES = sizeof( lineVertices ) / sizeof( lineVertices[0] );
	renderer->DrawVertexArray( NUM_VERTICES, lineVertices );
}


//-----------------------------------------------------------------------------------------------
// Render a ring as 64 small lines
//
void DrawRing2D( RenderContext* renderer, const Vec2& center, float radius, const Rgba8& color, float thickness )
{
	constexpr float NUM_SIDES = 64.f;
	constexpr float DEG_PER_SIDE = 360.f / NUM_SIDES;

	for ( int sideIndex = 0; sideIndex < NUM_SIDES; ++sideIndex )
	{
		float thetaDeg = DEG_PER_SIDE * (float)sideIndex;
		float theta2Deg = DEG_PER_SIDE * (float)( sideIndex + 1 );

		Vec2 start( radius * CosDegrees( thetaDeg ),
					radius * SinDegrees( thetaDeg ) );

		Vec2 end( radius * CosDegrees( theta2Deg ),
				  radius * SinDegrees( theta2Deg ) );

		// Translate start and end to be relative to the center of the ring
		start += center;
		end += center;

		DrawLine2D( renderer, start, end, color, thickness );
	}
}


//-----------------------------------------------------------------------------------------------
void DrawDisc2D( RenderContext* renderer, const Vec2& center, float radius, const Rgba8& color )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForArc( vertices, center, radius, 360.f, 0.f, color );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawCapsule2D( RenderContext* renderer, const Capsule2& capsule, const Rgba8& color )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForCapsule2D( vertices, capsule, color );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawAABB2( RenderContext* renderer, const AABB2& box, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForAABB2D( vertices, box, tint );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawAABB2WithDepth( RenderContext* renderer, const AABB2& box, float zDepth, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForAABB2DWithDepth( vertices, box, zDepth, tint );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawOBB2( RenderContext* renderer, const OBB2& box, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForOBB2D( vertices, box, tint );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
// TODO: This doesn't draw at correct position
void DrawAABB2Outline( RenderContext* renderer, const Vec2& center, const AABB2& box, const Rgba8& tint, float thickness )
{
	Vec2 bottomLeft( box.mins + center );
	Vec2 bottomRight( Vec2( box.maxs.x, box.mins.y ) + center );
	Vec2 topLeft( Vec2( box.mins.x, box.maxs.y ) + center );
	Vec2 topRight( box.maxs + center );

	DrawLine2D( renderer, bottomLeft, bottomRight, tint, thickness );
	DrawLine2D( renderer, bottomLeft, topLeft, tint, thickness );
	DrawLine2D( renderer, topLeft, topRight, tint, thickness );
	DrawLine2D( renderer, topRight, bottomRight, tint, thickness );
}


//-----------------------------------------------------------------------------------------------
void DrawOBB2Outline( RenderContext* renderer, const Vec2& center, const OBB2& box, const Rgba8& tint, float thickness )
{
	Vec2 boxHalfWidth( box.m_halfDimensions.x * box.m_iBasis );
	Vec2 boxHalfHeight( box.m_halfDimensions.y * box.GetJBasisNormal() );

	Vec2 topRight( center + box.m_center + boxHalfWidth + boxHalfHeight );
	Vec2 topLeft( center + box.m_center - boxHalfWidth + boxHalfHeight );
	Vec2 bottomLeft( center + box.m_center - boxHalfWidth - boxHalfHeight );
	Vec2 bottomRight( center + box.m_center + boxHalfWidth - boxHalfHeight );

	DrawLine2D( renderer, bottomLeft, bottomRight, tint, thickness );
	DrawLine2D( renderer, bottomLeft, topLeft, tint, thickness );
	DrawLine2D( renderer, topLeft, topRight, tint, thickness );
	DrawLine2D( renderer, topRight, bottomRight, tint, thickness );
}


//-----------------------------------------------------------------------------------------------
void DrawPolygon2( RenderContext* renderer, const std::vector<Vec2>& vertexPositions, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForPolygon2( vertices, vertexPositions, tint );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawPolygon2( RenderContext* renderer, const Polygon2& polygon2, const Rgba8& tint )
{
	DrawPolygon2( renderer, polygon2.GetPoints(), tint );
}


//-----------------------------------------------------------------------------------------------
void DrawPolygon2Outline( RenderContext* renderer, const std::vector<Vec2>& vertexPositions, const Rgba8& tint, float thickness )
{
	int numVertices = (int)vertexPositions.size();

	if ( numVertices < 3 )
	{
		g_devConsole->PrintString( Stringf( "Tried to draw a Polygon2Outline with %d vertices. At least 3 vertices are required.", numVertices ), Rgba8::YELLOW );
		return;
	}

	for ( int vertexPosIdx = 0; vertexPosIdx < numVertices - 1; ++vertexPosIdx )
	{
		int nextVertexIdx = vertexPosIdx + 1;
		DrawLine2D( renderer, vertexPositions[vertexPosIdx], vertexPositions[nextVertexIdx], tint, thickness );
	}

	// Connect last vertex to first
	int lastVertexIdx = numVertices - 1;
	DrawLine2D( renderer, vertexPositions[lastVertexIdx], vertexPositions[0], tint, thickness );
}


//-----------------------------------------------------------------------------------------------
void DrawPolygon2Outline( RenderContext* renderer, const Polygon2& polygon2, const Rgba8& tint, float thickness )
{
	DrawPolygon2Outline( renderer, polygon2.GetPoints(), tint, thickness );
}


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
void AppendVertsForPlaneMesh( std::vector<Vertex_PCU>& vertexArray, const Vec3& mins, const Vec2& dimensions, int horizontalCuts, int verticalCuts, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	float sectionWidth = dimensions.x / ( (float)verticalCuts + 1.f );
	float sectionHeight = dimensions.y / ( (float)horizontalCuts + 1.f );

	int numVertices = ( verticalCuts + 1 ) * ( horizontalCuts + 1 );
	vertexArray.reserve( numVertices );

	for ( int yIdx = 0; yIdx < horizontalCuts; ++yIdx )
	{
		for ( int xIdx = 0; xIdx < verticalCuts; ++xIdx )
		{
			vertexArray.push_back( Vertex_PCU( mins + Vec3( sectionWidth * xIdx, sectionHeight * yIdx, 0.f ), tint, uvAtMins ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void AppendIndicesForPlaneMesh( std::vector<uint>& indices )
{

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

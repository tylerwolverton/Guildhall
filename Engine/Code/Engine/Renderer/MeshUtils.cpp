#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ObjLoader.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Polygon2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/DebugRender.hpp"


//-----------------------------------------------------------------------------------------------
void DrawLine2D( RenderContext* renderer, 
				 const Vec2& start, const Vec2& end, 
				 const Rgba8& color, float thickness )
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
void DrawRing2D( RenderContext* renderer, 
				 const Vec2& center, float radius, 
				 const Rgba8& color, float thickness )
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
void DrawDisc2D( RenderContext* renderer, 
				 const Vec2& center, float radius, 
				 const Rgba8& color )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForArc( vertices, center, radius, 360.f, 0.f, color );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawCapsule2D( RenderContext* renderer, 
					const Capsule2& capsule, 
					const Rgba8& color )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForCapsule2D( vertices, capsule, color );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawAABB2( RenderContext* renderer, 
				const AABB2& box, 
				const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForAABB2D( vertices, box, tint );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawAABB2WithDepth( RenderContext* renderer, 
						 const AABB2& box, float zDepth, 
						 const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForAABB2DWithDepth( vertices, box, zDepth, tint );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawOBB2( RenderContext* renderer, 
			   const OBB2& box, 
			   const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForOBB2D( vertices, box, tint );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
// TODO: This doesn't draw at correct position
void DrawAABB2Outline( RenderContext* renderer, 
					   const Vec2& center, const AABB2& box, 
					   const Rgba8& tint, float thickness )
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
void DrawOBB2Outline( RenderContext* renderer, 
					  const Vec2& center, const OBB2& box, 
					  const Rgba8& tint, float thickness )
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
void DrawPolygon2( RenderContext* renderer, 
				   const std::vector<Vec2>& vertexPositions, 
				   const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForPolygon2( vertices, vertexPositions, tint );

	renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DrawPolygon2( RenderContext* renderer, 
				   const Polygon2& polygon2, 
				   const Rgba8& tint )
{
	DrawPolygon2( renderer, polygon2.GetPoints(), tint );
}


//-----------------------------------------------------------------------------------------------
void DrawPolygon2Outline( RenderContext* renderer, 
						  const std::vector<Vec2>& vertexPositions, 
						  const Rgba8& tint, float thickness )
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
void DrawPolygon2Outline( RenderContext* renderer, 
						  const Polygon2& polygon2, 
						  const Rgba8& tint, float thickness )
{
	DrawPolygon2Outline( renderer, polygon2.GetPoints(), tint, thickness );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForArc( std::vector<Vertex_PCU>& vertexArray, 
						const Vec2& center, float radius, 
						float arcAngleDegrees, float startOrientationDegrees, 
						const Rgba8& tint )
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
void AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, 
						   const AABB2& spriteBounds, 
						   const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	vertexArray.push_back( Vertex_PCU( spriteBounds.mins, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( Vec2( spriteBounds.maxs.x, spriteBounds.mins.y ), tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( spriteBounds.maxs, tint, uvAtMaxs ) );

	vertexArray.push_back( Vertex_PCU( spriteBounds.mins, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( spriteBounds.maxs, tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( Vec2( spriteBounds.mins.x, spriteBounds.maxs.y ), tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForAABB3D( std::vector<Vertex_PCU>& vertexArray, 
						   const AABB3& bounds, 
						   const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec3 mins = bounds.mins;
	Vec3 maxs = bounds.maxs;
	
	std::vector<Vec3> corners;

	// Front 4 points
	corners.emplace_back( mins );
	corners.emplace_back( maxs.x, mins.y, mins.z );
	corners.emplace_back( mins.x, maxs.y, mins.z );
	corners.emplace_back( maxs.x, maxs.y, mins.z );
	
	// Back 4 points ( from front perspective for directions )	
	corners.emplace_back( mins.x, mins.y, maxs.z );
	corners.emplace_back( maxs.x, mins.y, maxs.z );
	corners.emplace_back( mins.x, maxs.y, maxs.z );
	corners.emplace_back( maxs );

	AppendVertsFor3DBox( vertexArray, corners, tint, tint, uvAtMins, uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, 
						  const OBB2& spriteBounds, 
						  const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
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
void AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, 
						  const Vec2& bottomLeft, const Vec2& bottomRight, const Vec2& topLeft, const Vec2& topRight, 
						  const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	vertexArray.push_back( Vertex_PCU( bottomLeft, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( bottomRight, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( topRight, tint, uvAtMaxs ) );

	vertexArray.push_back( Vertex_PCU( bottomLeft, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( topRight, tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( topLeft, tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForOBB3D( std::vector<Vertex_PCU>& vertexArray, 
						  const OBB3& bounds, 
						  const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	AppendVertsForOBB3D( vertexArray, bounds, tint, tint, uvAtMins, uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForOBB3D( std::vector<Vertex_PCU>& vertexArray, 
						  const OBB3& bounds, 
						  const Rgba8& startTint, const Rgba8& endTint, 
						  const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec3 corners[8];

	bounds.GetCornerPositions( corners );

	AppendVertsFor3DBox( vertexArray, 8, corners, startTint, endTint, uvAtMins, uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, 
							  const Capsule2& capsule, 
							  const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
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
void AppendVertsForPolygon2( std::vector<Vertex_PCU>& vertexArray, 
							 const std::vector<Vec2>& vertexPositions, 
							 const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
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
void AppendVertsForCubeMesh( std::vector<Vertex_PCU>& vertexArray, 
							 const Vec3& center, float sideLength, 
							 const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
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
void AppendVertsAndIndicesForCubeMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, 
									   const Vec3& center, float sideLength, 
									   const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	AppendVertsForCubeMesh( vertexArray, center, sideLength, tint, uvAtMins, uvAtMaxs );
	AppendIndicesForCubeMesh( indices );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsFor3DBox( std::vector<Vertex_PCU>& vertexArray, std::vector<Vec3>& corners, 
						  const Rgba8& frontTint, const Rgba8& backTint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	AppendVertsFor3DBox( vertexArray, (int)corners.size(), &corners[0], frontTint, backTint, uvAtMins, uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsFor3DBox( std::vector<Vertex_PCU>& vertexArray, int cornerCount, Vec3* corners, 
						  const Rgba8& frontTint, const Rgba8& backTint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	GUARANTEE_OR_DIE( cornerCount == 8, "Must call AppendVertsFor3DBox with exactly 8 points" );

	vertexArray.reserve( 24 );
	// Front
	vertexArray.push_back( Vertex_PCU( corners[0], frontTint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( corners[1], frontTint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[2], frontTint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[3], frontTint, uvAtMaxs ) );

	// Right
	vertexArray.push_back( Vertex_PCU( corners[1], frontTint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( corners[5], backTint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[3], frontTint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[7], backTint, uvAtMaxs ) );

	// Back
	vertexArray.push_back( Vertex_PCU( corners[4], backTint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[5], backTint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( corners[6], backTint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( corners[7], backTint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );

	// Left
	vertexArray.push_back( Vertex_PCU( corners[4], backTint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( corners[0], frontTint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[6], backTint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[2], frontTint, uvAtMaxs ) );

	// Top
	vertexArray.push_back( Vertex_PCU( corners[2], frontTint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( corners[3], frontTint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[6], backTint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[7], backTint, uvAtMaxs ) );

	// Bottom
	vertexArray.push_back( Vertex_PCU( corners[0], frontTint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( corners[1], frontTint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( corners[4], backTint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( corners[5], backTint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
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
void AppendVertsForPlaneMesh( std::vector<Vertex_PCU>& vertexArray, 
							  const Vec3& mins, const Vec2& dimensions, 
							  int horizontalSlices, int verticalSlices, 
							  const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	float sectionWidth = dimensions.x / ( (float)verticalSlices + 1.f );
	float sectionHeight = dimensions.y / ( (float)horizontalSlices + 1.f );

	int numVertices = ( verticalSlices + 1 ) * ( horizontalSlices + 1 );
	vertexArray.reserve( numVertices );

	Vec2 uvRange( uvAtMaxs - uvAtMins );
	Vec2 uvSteps( uvRange.x / (float)( verticalSlices ), uvRange.y / (float)( horizontalSlices ) );

	for ( int yIdx = 0; yIdx < horizontalSlices + 1; ++yIdx )
	{
		for ( int xIdx = 0; xIdx < verticalSlices + 1; ++xIdx )
		{
			Vec2 uvs( uvAtMins.x + uvSteps.x * xIdx, uvAtMins.y + uvSteps.y * yIdx );
			vertexArray.push_back( Vertex_PCU( mins + Vec3( sectionWidth * xIdx, sectionHeight * yIdx, 0.f ), tint, uvs ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void AppendIndicesForPlaneMesh( std::vector<uint>& indices, 
								int horizontalSlices, int verticalSlices )
{
	int numIndices = ( verticalSlices + 1 ) * ( horizontalSlices + 1 ) * 6;
	indices.reserve( numIndices );

	for ( int yIdx = 0; yIdx < horizontalSlices; ++yIdx )
	{
		for ( int xIdx = 0; xIdx < verticalSlices; ++xIdx )
		{
			int start = yIdx * ( verticalSlices + 1 ) + xIdx;
			indices.push_back( start );
			indices.push_back( start + 1 );
			indices.push_back( start + verticalSlices + 2 );

			indices.push_back( start );
			indices.push_back( start + verticalSlices + 2 );
			indices.push_back( start + verticalSlices + 1 );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForSphereMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, 
										 const Vec3& center, float radius, 
										 int horizontalSlices, int verticalSlices, 
										 const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	int numVertices = ( verticalSlices + 1 ) * ( horizontalSlices + 1 );
	vertexArray.reserve( numVertices );	

	Vec2 uvRange( uvAtMaxs - uvAtMins );
	Vec2 uvSteps( uvRange.x / (float)( verticalSlices ), uvRange.y / (float)( horizontalSlices ) );

	for ( int yIdx = 0; yIdx < horizontalSlices + 1; ++yIdx )
	{
		float phi = RangeMapFloat( 0.f, (float)horizontalSlices, -90.f, 90.f, (float)yIdx );

		for ( int xIdx = 0; xIdx < verticalSlices + 1; ++xIdx )
		{
			float theta = RangeMapFloat( 0.f, (float)verticalSlices, 0.f, 360.f, (float)xIdx );

			float cosPhi = CosDegrees( phi );

			float posX = cosPhi * CosDegrees( theta );
			float posY = SinDegrees( phi );
			float posZ = -cosPhi * SinDegrees( theta );

			Vec3 position = center + Vec3( posX, posY, posZ) * radius;

			Vec2 uvs( uvAtMins.x + ( uvSteps.x * xIdx ), uvAtMins.y + ( uvSteps.y * yIdx ) );

			vertexArray.push_back( Vertex_PCU( position, tint, uvs ) );
		}
	}

	AppendIndicesForSphereMesh( indices, horizontalSlices, verticalSlices );
}


//-----------------------------------------------------------------------------------------------
void AppendIndicesForSphereMesh( std::vector<uint>& indices, int horizontalSlices, int verticalSlices )
{
	int numIndices = ( verticalSlices + 1 ) * ( horizontalSlices + 1 ) * 6;
	indices.reserve( numIndices );

	for ( uint yIdx = 0; yIdx < (uint)horizontalSlices; ++yIdx )
	{
		for ( uint xIdx = 0; xIdx < (uint)verticalSlices; ++xIdx )
		{
			uint start = yIdx * ( (uint)verticalSlices + 1 ) + xIdx;

			uint bottomLeft = start;
			uint bottomRight = start + 1;
			uint topLeft = start + verticalSlices + 1;
			uint topRight = start + verticalSlices + 2;

			indices.push_back( bottomLeft );
			indices.push_back( bottomRight );
			indices.push_back( topRight );

			indices.push_back( bottomLeft );
			indices.push_back( topRight );
			indices.push_back( topLeft );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForCylinderMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, 
										   const Vec3& p0, const Vec3& p1, float radius1, float radius2, 
										   const Rgba8& tint, int numSides, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	AppendVertsAndIndicesForCylinderMesh( vertexArray, indices, p0, p1, radius1, radius2, tint, tint, numSides, uvAtMins, uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForCylinderMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, 
										   const Vec3& p0, const Vec3& p1, 
										   float radius1, float radius2, 
										   const Rgba8& startTint, const Rgba8& endTint, int numSides, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	// Not texturing cylinders for now
	UNUSED( uvAtMaxs );

	GUARANTEE_OR_DIE( numSides > 2, "Can't draw a cylinder with less than 3 sides" );

	Mat44 lookAt = MakeLookAtMatrix( p0, p1 );

	std::vector<Vec3> localDiscPoints;

	const float degreesPerSide = 360.f / numSides;
	float currentDegrees = 0.f;
	for ( int pointCount = 0; pointCount < numSides; ++pointCount )
	{
		Vec3 newPointLocation = lookAt.GetIBasis3D() * CosDegrees( currentDegrees ) + lookAt.GetJBasis3D() * SinDegrees( currentDegrees );
		localDiscPoints.push_back( newPointLocation );

		currentDegrees += degreesPerSide;
	}

	uint numLocalDiscPoints = (uint)localDiscPoints.size();
	for ( uint pointIdx = 0; pointIdx < numLocalDiscPoints; ++pointIdx )
	{
		Vec3 startPoint = p0 + localDiscPoints[pointIdx] * radius1;
		vertexArray.push_back( Vertex_PCU( startPoint, startTint, uvAtMins ) );
	}

	for ( uint pointIdx = 0; pointIdx < numLocalDiscPoints; ++pointIdx )
	{
		Vec3 endPoint = p1 + localDiscPoints[pointIdx] * radius2;
		vertexArray.push_back( Vertex_PCU( endPoint, endTint, uvAtMins ) );
	}

	// Add indices for center
	for ( uint vertexNum = 0; vertexNum < numLocalDiscPoints; ++vertexNum )
	{
		uint index0 = vertexNum;
		uint index1 = numLocalDiscPoints + vertexNum;
		uint index2 = vertexNum + 1;
		uint index3 = numLocalDiscPoints + vertexNum + 1;
		if ( vertexNum == numLocalDiscPoints - 1 )
		{
			index2 = 0;
			index3 = numLocalDiscPoints;
		}

		indices.push_back( index0 );
		indices.push_back( index1 );
		indices.push_back( index3 );

		indices.push_back( index0 );
		indices.push_back( index3 );
		indices.push_back( index2 );
	}

	// Add indices for caps/ends
	uint startingIndex = 0;
	for ( uint vertexNum = 1; vertexNum < numLocalDiscPoints - 1; ++vertexNum )
	{
		indices.push_back( startingIndex );
		indices.push_back( startingIndex + vertexNum );
		indices.push_back( startingIndex + vertexNum + 1 );
	}

	startingIndex = numLocalDiscPoints;
	for ( uint vertexNum = 1; vertexNum < numLocalDiscPoints - 1; ++vertexNum )
	{
		indices.push_back( startingIndex );
		indices.push_back( startingIndex + vertexNum );
		indices.push_back( startingIndex + vertexNum + 1 );
	}
}


//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForConeMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, 
									   const Vec3& p0, const Vec3& p1, float radius,
									   const Rgba8& tint, int numSides, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	AppendVertsAndIndicesForCylinderMesh( vertexArray, indices, p0, p1, radius, 0.f, tint, numSides, uvAtMins , uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForConeMesh( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indices, 
									   const Vec3& p0, const Vec3& p1, float radius, 
									   const Rgba8& startTint, const Rgba8& endTint, int numSides, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	AppendVertsAndIndicesForCylinderMesh( vertexArray, indices, p0, p1, radius, 0.f, startTint, endTint, numSides, uvAtMins, uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForAABB2DWithDepth( std::vector<Vertex_PCU>& vertexArray, 
									const AABB2& spriteBounds, float zDepth, 
									const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
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


//-----------------------------------------------------------------------------------------------
// Vertex_PCUTBN
//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForQuad( std::vector<Vertex_PCUTBN>& vertexArray, std::vector<uint>& indices,
								   const AABB2& bounds, 
								   const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec3 mins( bounds.mins, 0.f );
	Vec3 maxs( bounds.maxs, 0.f );

	Vec3 v0 = mins;
	Vec3 v1( bounds.maxs.x, bounds.mins.y, 0.f );
	Vec3 v2( bounds.mins.x, bounds.maxs.y, 0.f );
	Vec3 v3 = maxs;

	Vec3 right = v1 - v0;
	Vec3 up = v2 - v0;

	Vec3 normal = CrossProduct3D( right, up ).GetNormalized();
	Vec3 tangent = right.GetNormalized();

	Vec3 arrowPos = mins + ( maxs - mins ) * .5f;

	vertexArray.push_back( Vertex_PCUTBN( v0, tint, uvAtMins, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( v1, tint, Vec2( uvAtMaxs.x, uvAtMins.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( v3, tint, uvAtMaxs, normal, tangent ) );

	vertexArray.push_back( Vertex_PCUTBN( v0, tint, uvAtMins, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( v3, tint, uvAtMaxs, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( v2, tint, Vec2( uvAtMins.x, uvAtMaxs.y ), normal, tangent ) );

	indices.push_back( 0 );
	indices.push_back( 1 );
	indices.push_back( 2 );

	indices.push_back( 3 );
	indices.push_back( 4 );
	indices.push_back( 5 );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForSphereMesh( std::vector<Vertex_PCUTBN>& vertexArray, std::vector<uint>& indices, 
										 const Vec3& center, float radius, 
										 int horizontalSlices, int verticalSlices, 
										 const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	int numVertices = ( verticalSlices + 1 ) * ( horizontalSlices + 1 );
	vertexArray.reserve( numVertices );

	Vec2 uvRange( uvAtMaxs - uvAtMins );
	Vec2 uvSteps( uvRange.x / (float)( verticalSlices ), uvRange.y / (float)( horizontalSlices ) );

	for ( int yIdx = 0; yIdx < horizontalSlices + 1; ++yIdx )
	{
		float phi = RangeMapFloat( 0.f, (float)horizontalSlices, -90.f, 90.f, (float)yIdx );
		float cosPhi = CosDegrees( phi );

		for ( int xIdx = 0; xIdx < verticalSlices + 1; ++xIdx )
		{
			float theta = RangeMapFloat( 0.f, (float)verticalSlices, 0.f, 360.f, (float)xIdx );
			float cosTheta = CosDegrees( theta );
			float sinTheta = SinDegrees( theta );

			float posX = cosPhi * cosTheta;
			float posY = SinDegrees( phi );
			float posZ = -cosPhi * sinTheta;

			Vec3 position = center + Vec3( posX, posY, posZ ) * radius;

			float tanPosX = -sinTheta;
			float tanPosY = 0.f;
			float tanPosZ =  -cosTheta;
			
			Vec3 tanPosition( tanPosX, tanPosY, tanPosZ );

			Vec2 uvs( uvAtMins.x + ( uvSteps.x * xIdx ), uvAtMins.y + ( uvSteps.y * yIdx ) );

			Vec3 normal = ( position - center );
			normal.Normalize();

			Vec3 tangent = tanPosition;
			tangent.Normalize();
			
			vertexArray.push_back( Vertex_PCUTBN( position, tint, uvs, normal, tangent ) );
		}
	}

	AppendIndicesForSphereMesh( indices, horizontalSlices, verticalSlices );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsForCubeMesh( std::vector<Vertex_PCUTBN>& vertexArray, 
							 const Vec3& center, float sideLength, 
							 const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
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
	Vec3 right = vert1 - vert0;
	Vec3 up = vert2 - vert0;

	Vec3 normal = CrossProduct3D( right, up ).GetNormalized();
	Vec3 tangent = right.GetNormalized();
	
	vertexArray.push_back( Vertex_PCUTBN( vert0, tint, uvAtMins, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert1, tint, Vec2( uvAtMaxs.x, uvAtMins.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert2, tint, Vec2( uvAtMins.x, uvAtMaxs.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert3, tint, uvAtMaxs, normal, tangent ) );

	// Right
	right = vert5 - vert1;
	up = vert3 - vert1;

	normal = CrossProduct3D( right, up ).GetNormalized();
	tangent = right.GetNormalized();

	vertexArray.push_back( Vertex_PCUTBN( vert1, tint, uvAtMins, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert5, tint, Vec2( uvAtMaxs.x, uvAtMins.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert3, tint, Vec2( uvAtMins.x, uvAtMaxs.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert7, tint, uvAtMaxs, normal, tangent ) );

	// Back
	right = vert4 - vert5;
	up = vert6 - vert4;

	normal = CrossProduct3D( right, up ).GetNormalized();
	tangent = right.GetNormalized();
	
	vertexArray.push_back( Vertex_PCUTBN( vert4, tint, Vec2( uvAtMaxs.x, uvAtMins.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert5, tint, uvAtMins, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert6, tint, uvAtMaxs, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert7, tint, Vec2( uvAtMins.x, uvAtMaxs.y ), normal, tangent ) );

	// Left
	right = vert0 - vert4;
	up = vert6 - vert4;

	normal = CrossProduct3D( right, up ).GetNormalized();
	tangent = right.GetNormalized();
	
	vertexArray.push_back( Vertex_PCUTBN( vert4, tint, uvAtMins, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert0, tint, Vec2( uvAtMaxs.x, uvAtMins.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert6, tint, Vec2( uvAtMins.x, uvAtMaxs.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert2, tint, uvAtMaxs, normal, tangent ) );

	// Top
	right = vert3 - vert2;
	up = vert6 - vert2;

	normal = CrossProduct3D( right, up ).GetNormalized();
	tangent = right.GetNormalized();
	
	vertexArray.push_back( Vertex_PCUTBN( vert2, tint, uvAtMins, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert3, tint, Vec2( uvAtMaxs.x, uvAtMins.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert6, tint, Vec2( uvAtMins.x, uvAtMaxs.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert7, tint, uvAtMaxs, normal, tangent ) );

	// Bottom
	right = vert5 - vert4;
	up = vert0 - vert4;

	normal = CrossProduct3D( right, up ).GetNormalized();
	tangent = right.GetNormalized();

	vertexArray.push_back( Vertex_PCUTBN( vert0, tint, Vec2( uvAtMaxs.x, uvAtMins.y ), normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert1, tint, uvAtMins, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert4, tint, uvAtMaxs, normal, tangent ) );
	vertexArray.push_back( Vertex_PCUTBN( vert5, tint, Vec2( uvAtMins.x, uvAtMaxs.y ), normal, tangent ) );
}


//-----------------------------------------------------------------------------------------------
void AppendVertsAndIndicesForCubeMesh( std::vector<Vertex_PCUTBN>& vertexArray, std::vector<uint>& indices, 
									   const Vec3& center, float sideLength, 
									   const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	AppendVertsForCubeMesh( vertexArray, center, sideLength, tint, uvAtMins, uvAtMaxs );
	AppendIndicesForCubeMesh( indices );
}


//-----------------------------------------------------------------------------------------------
// Obj loading
//-----------------------------------------------------------------------------------------------
void AppendVertsForObjMeshFromFile( std::vector<Vertex_PCUTBN>& vertices,
									std::string objFileName,
									const MeshImportOptions& options )
{
	if ( options.generateTangents )
	{
		GUARANTEE_OR_DIE( options.generateNormals, "During obj load tangents were requested but normals were not." );
	}

	bool fileHadNormals = false;
	ObjLoader::LoadFromFile( vertices, objFileName, fileHadNormals );

	if ( options.invertVs )
	{
		ObjLoader::InvertVertVs( vertices );
	}

	// Only generate normals if they are requested and weren't loaded from file
	if ( options.generateNormals 
		 && !fileHadNormals )
	{
		ObjLoader::GenerateVertNormals( vertices );
	}

	if ( options.generateTangents )
	{
		ObjLoader::GenerateVertTangents( vertices );
	}

	if ( options.invertWindingOrder )
	{
		ObjLoader::InvertVertWindingOrder( vertices );
	}
}

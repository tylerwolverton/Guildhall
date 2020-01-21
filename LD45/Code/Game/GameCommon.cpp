#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
void DrawLine(const Vec2& start, const Vec2& end, const Rgba8& color, float thickness)
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

	Vertex_PCU lineVertexes[] =
	{
		Vertex_PCU( startRight, color ),
		Vertex_PCU( endRight, color ),
		Vertex_PCU( endLeft, color ),

		Vertex_PCU( startRight, color ),
		Vertex_PCU( endLeft, color ),
		Vertex_PCU( startLeft, color )
	};

	constexpr int NUM_VERTEXES = sizeof( lineVertexes ) / sizeof( lineVertexes[0] );
	g_theRenderer->DrawVertexArray( NUM_VERTEXES, lineVertexes );
}


//-----------------------------------------------------------------------------------------------
// Render a ring as 64 small lines
//
void DrawRing(const Vec2& center, float radius, const Rgba8& color, float thickness)
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

		DrawLine( start, end, color, thickness );
	}
}

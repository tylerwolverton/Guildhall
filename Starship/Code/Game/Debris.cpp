#include "Debris.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Debris::Debris(Game* theGame, const Vec2& position, const Vec2& velocity, const Rgba8& color)
	: Entity( theGame, position )
	, m_color( color )
{
	constexpr int halfAlpha = (int)( (float)255 * .5f );
	m_color.a = halfAlpha;

	m_physicsRadius = DEBRIS_PHYSICS_RADIUS;
	m_cosmeticRadius = DEBRIS_COSMETIC_RADIUS;
	PopulateVertexes();

	m_velocity = -velocity;
	m_angularVelocity = m_game->m_randNumGen->RollRandomFloatInRange( -300.f, 300.f );
}


//-----------------------------------------------------------------------------------------------
Debris::~Debris()
{
}


//-----------------------------------------------------------------------------------------------
void Debris::Update(float deltaSeconds)
{
	m_ageSeconds += deltaSeconds;
	if ( m_ageSeconds > DEBRIS_LIFESPAN_SECONDS )
	{
		Die();
		return;
	}

	// Map age to alpha (.5 - 0)
	m_color.a = (unsigned char)( (float)255 * RangeMapFloat(0.f, DEBRIS_LIFESPAN_SECONDS, .5f, 0.f, m_ageSeconds) );

	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Debris::Render() const
{
	// Make a copy of the local verts and transform so the original is preserved
	Vertex_PCU copyOfLocalVerts[NUM_DEBRIS_VERTS];
	for ( int vertexIndex = 0; vertexIndex < NUM_DEBRIS_VERTS; ++vertexIndex )
	{
		copyOfLocalVerts[vertexIndex] = m_localRenderingVertexes[vertexIndex];
		copyOfLocalVerts[vertexIndex].m_color.a = m_color.a;
	}

	constexpr int NUM_VERTEXES = sizeof( copyOfLocalVerts ) / sizeof( copyOfLocalVerts[0] );
	Vertex_PCU::TransformVertexArray( copyOfLocalVerts, NUM_VERTEXES, 1.f, m_orientationDegrees, m_position );

	g_renderer->DrawVertexArray( NUM_VERTEXES, copyOfLocalVerts );
}


//-----------------------------------------------------------------------------------------------
void Debris::Die()
{
	m_isGarbage = true;

	Entity::Die();
}


//-----------------------------------------------------------------------------------------------
void Debris::PopulateVertexes()
{
	// Get random float between physics and cosmetic radius
	Vec2 zero2D = Vec2( 0.f, 0.f );

	constexpr float deltaDegrees = 360.f / ( (float)NUM_DEBRIS_VERTS / 3.f );
	float curAngleDegrees = 0;

	for ( int vertexIndex = 0; vertexIndex < NUM_DEBRIS_VERTS; )
	{
		m_localRenderingVertexes[vertexIndex] = Vertex_PCU( zero2D, m_color );

		// The first triangle requires both outer points to be calculated
		if ( vertexIndex == 0 )
		{
			float radius = m_game->m_randNumGen->RollRandomFloatInRange( m_physicsRadius, m_cosmeticRadius );
			m_localRenderingVertexes[vertexIndex + 1] = Vertex_PCU( Vec2( radius * CosDegrees( curAngleDegrees ),
																		  radius * SinDegrees( curAngleDegrees ) ),
																	m_color );
			curAngleDegrees += deltaDegrees;
		}
		// Use the final point in the last triangle as the second point in this triangle
		// so that the 2 triangles connect to form a contiguous outline for the asteroid
		else
		{
			m_localRenderingVertexes[vertexIndex + 1] = m_localRenderingVertexes[vertexIndex - 1];
		}

		float radius = m_game->m_randNumGen->RollRandomFloatInRange( m_physicsRadius, m_cosmeticRadius );
		m_localRenderingVertexes[vertexIndex + 2] = Vertex_PCU( Vec2( radius * CosDegrees( curAngleDegrees ),
																	  radius * SinDegrees( curAngleDegrees ) ),
																m_color );

		curAngleDegrees += deltaDegrees;
		// Advance to next set of 3 vertexes
		vertexIndex += 3;
	}
}

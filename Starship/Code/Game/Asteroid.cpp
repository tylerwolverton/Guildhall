#include "Asteroid.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Asteroid::Asteroid(Game* theGame, const Vec2& position)
	: Entity(theGame, position)
{
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_numDebrisPieces = NUM_ASTEROID_DEBRIS_PIECES;
	m_health = MAX_ASTEROID_HEALTH;
	m_mainColor = Rgba8(100, 100, 100);
	
	PopulateVertexes();

	m_velocity = ASTEROID_SPEED * Vec2::MakeFromPolarDegrees( m_game->m_randNumGen->RollRandomFloatLessThan( 360.f ) );
	m_angularVelocity = m_game->m_randNumGen->RollRandomFloatInRange( -200.f, 200.f );
}


//-----------------------------------------------------------------------------------------------
void Asteroid::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );

	if ( IsOffScreen() )
	{
		Die();
	}
}


//-----------------------------------------------------------------------------------------------
void Asteroid::Render() const
{
	unsigned char flashAmount = (unsigned char)RangeMapFloat( 1.f, 0.f, 255.f, 0.f, m_hitFlashTimer );
	Rgba8 rockColor = Rgba8( m_mainColor.r + flashAmount,
							 m_mainColor.g + flashAmount,
							 m_mainColor.b + flashAmount );

	// Make a copy of the local verts and transform so the original is preserved
	Vertex_PCU copyOfLocalVerts[NUM_ASTEROID_VERTS];
	for ( int vertexIndex = 0; vertexIndex < NUM_ASTEROID_VERTS; ++vertexIndex )
	{
		copyOfLocalVerts[vertexIndex] = m_localRenderingVertexes[vertexIndex];
		copyOfLocalVerts[vertexIndex].m_color = rockColor;
	}

	constexpr int NUM_VERTEXES = sizeof( copyOfLocalVerts ) / sizeof( copyOfLocalVerts[0] );
	Vertex_PCU::TransformVertexArray( copyOfLocalVerts, NUM_VERTEXES, 1.f, m_orientationDegrees, m_position );

	g_renderer->DrawVertexArray( NUM_VERTEXES, copyOfLocalVerts );
}


//-----------------------------------------------------------------------------------------------
void Asteroid::Die()
{
	m_isGarbage = true;

	Entity::Die();
}


//-----------------------------------------------------------------------------------------------
void Asteroid::PopulateVertexes()
{
	Vec2 zero2D = Vec2( 0.f, 0.f );
	
	constexpr float deltaDegrees = 360.f / ( (float)NUM_ASTEROID_VERTS / 3.f );
	float curAngleDegrees = 0;

	for ( int vertexIndex = 0; vertexIndex < NUM_ASTEROID_VERTS; )
	{
		m_localRenderingVertexes[vertexIndex] = Vertex_PCU( zero2D, m_mainColor );

		// The first triangle requires both outer points to be calculated
		if ( vertexIndex == 0 )
		{
			float radius = m_game->m_randNumGen->RollRandomFloatInRange( m_physicsRadius, m_cosmeticRadius );
			m_localRenderingVertexes[vertexIndex + 1] = Vertex_PCU( Vec2( radius * CosDegrees( curAngleDegrees ),
																		  radius * SinDegrees( curAngleDegrees ) ),
																   m_mainColor);
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
																	  radius * SinDegrees( curAngleDegrees )), 
															   m_mainColor);
				
		curAngleDegrees += deltaDegrees;
		// Advance to next set of 3 vertexes
		vertexIndex += 3;
	}
}

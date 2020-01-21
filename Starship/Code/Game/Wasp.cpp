#include "Wasp.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Wasp::Wasp( Game* theGame, const Vec2& position )
	: Entity( theGame, position )
{
	m_physicsRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;
	m_mainColor = Rgba8(200, 200, 75);
	m_numDebrisPieces = NUM_WASP_DEBRIS_PIECES;
	m_health = MAX_WASP_HEALTH;
}


//-----------------------------------------------------------------------------------------------
Wasp::~Wasp()
{
}


//-----------------------------------------------------------------------------------------------
void Wasp::Update( float deltaSeconds )
{
	Vec2 dirToMove = m_velocity.GetNormalized();

	// Update Wasps's velocity if player is alive
	Vec2 playerPos = m_game->GetPlayerPosition();
	if ( playerPos != Vec2( -1.f, -1.f ) )
	{
		dirToMove = GetNormalizedDirectionFromAToB( m_position, playerPos );

	}

	// Calculate and clamp velocity
	m_linearAcceleration = dirToMove * WASP_ACCELERATION;
	m_velocity += m_linearAcceleration * deltaSeconds;
	m_linearAcceleration = Vec2( 0.f, 0.f );
	m_velocity.ClampLength( WASP_MAX_SPEED );
	
	m_orientationDegrees = dirToMove.GetOrientationDegrees();

	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Wasp::Render() const
{
	unsigned char flashAmount = (unsigned char)RangeMapFloat( 1.f, 0.f, 255.f, 0.f, m_hitFlashTimer );
	Rgba8 waspColor = Rgba8( m_mainColor.r + flashAmount,
							 m_mainColor.g + flashAmount,
							 m_mainColor.b + flashAmount );

	Vertex_PCU waspVertexes[] =
	{
		// Top
		Vertex_PCU( Vec2( 1.f, 2.f ), waspColor ),
		Vertex_PCU( Vec2( 1.f, -2.f ), waspColor ),
		Vertex_PCU( Vec2( 2.f, 0.f ), waspColor ),

		// Middle
		Vertex_PCU( Vec2( -1.f, 1.f ), waspColor ),
		Vertex_PCU( Vec2( -1.f, -1.f ), waspColor ),
		Vertex_PCU( Vec2( 1.5f, 0.f ), waspColor ),

		// Bottom
		Vertex_PCU( Vec2( -2.f, 0.5f ), waspColor ),
		Vertex_PCU( Vec2( -2.f, -0.5f ), waspColor ),
		Vertex_PCU( Vec2( 0.5f, 0.f ), waspColor ),
	};

	constexpr int NUM_VERTEXES = sizeof( waspVertexes ) / sizeof( waspVertexes[0] );
	Vertex_PCU::TransformVertexArray( waspVertexes, NUM_VERTEXES, 1.f, m_orientationDegrees, m_position );

	g_renderer->DrawVertexArray( NUM_VERTEXES, waspVertexes );
}


//-----------------------------------------------------------------------------------------------
void Wasp::Die()
{
	m_isGarbage = true;
	
	Entity::Die();
}

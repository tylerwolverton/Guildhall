#include "Beetle.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Beetle::Beetle(Game* theGame, const Vec2& position)
	: Entity(theGame, position)
{
	m_physicsRadius = BEETLE_PHYSICS_RADIUS;
	m_cosmeticRadius = BEETLE_COSMETIC_RADIUS;
	m_mainColor = Rgba8(0, 200, 75);
	m_numDebrisPieces = NUM_BEETLE_DEBRIS_PIECES;
	m_health = MAX_BEETLE_HEALTH;
}


//-----------------------------------------------------------------------------------------------
Beetle::~Beetle()
{
}


//-----------------------------------------------------------------------------------------------
void Beetle::Update(float deltaSeconds)
{
	Vec2 dirToMove = m_velocity.GetNormalized();

	// Update Beetle's velocity if player is alive
	Vec2 playerPos = m_game->GetPlayerPosition();
	if ( playerPos != Vec2(-1.f, -1.f) )
	{
		dirToMove = GetNormalizedDirectionFromAToB(m_position, playerPos);

	}

	m_velocity = dirToMove * BEETLE_SPEED;
	m_orientationDegrees = dirToMove.GetOrientationDegrees();

	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Beetle::Render() const
{
	unsigned char flashAmount = (unsigned char)RangeMapFloat( 1.f, 0.f, 255.f, 0.f, m_hitFlashTimer );
	Rgba8 beetleColor = Rgba8 ( m_mainColor.r + flashAmount,
								m_mainColor.g + flashAmount,
								m_mainColor.b + flashAmount );

	Vertex_PCU beetleVertexes[] =
	{
		Vertex_PCU(Vec2(0.f, 1.f), beetleColor),
		Vertex_PCU(Vec2(1.f, 0.f), beetleColor),
		Vertex_PCU(Vec2(0.f, -1.f), beetleColor),

		Vertex_PCU(Vec2(-2.f, 0.f), beetleColor),
		Vertex_PCU(Vec2(0.f, -2.f), beetleColor),
		Vertex_PCU(Vec2(0.f, 2.f), beetleColor)
	};

	constexpr int NUM_VERTEXES = sizeof(beetleVertexes) / sizeof(beetleVertexes[0]);
	Vertex_PCU::TransformVertexArray(beetleVertexes, NUM_VERTEXES, 1.f, m_orientationDegrees, m_position);

	g_renderer->DrawVertexArray(NUM_VERTEXES, beetleVertexes);
}


//-----------------------------------------------------------------------------------------------
void Beetle::Die()
{
	m_isGarbage = true;

	Entity::Die();
}

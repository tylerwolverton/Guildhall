#include "StarburstBullet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
StarburstBullet::StarburstBullet(Game* theGame, const Vec2& position, const Vec2& forwardVector)
	: Bullet(theGame, position, forwardVector)
{
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_velocity = BULLET_SPEED * forwardVector;
	m_orientationDegrees = forwardVector.GetOrientationDegrees();
	m_mainColor = Rgba8(255, 100, 0);
	m_numDebrisPieces = NUM_BULLET_DEBRIS_PIECES;
	m_health = 1;
}


//-----------------------------------------------------------------------------------------------
void StarburstBullet::Update(float deltaSeconds)
{
	Entity::Update(deltaSeconds);

	if ( IsOffScreen() )
	{
		Die();
	}
}


//-----------------------------------------------------------------------------------------------
void StarburstBullet::Render() const
{
	Rgba8 noseColor(255, 255, 0);
	Rgba8 sideColor(255, 100, 0);
	Rgba8 tailColor(255, 100, 0, 0);

	Vertex_PCU bulletVertexes[] =
	{
		// Nose
		Vertex_PCU(Vec2(0.f, 0.5f), sideColor),
		Vertex_PCU(Vec2(0.5f, 0.f), noseColor),
		Vertex_PCU(Vec2(0.f, -0.5f), sideColor),

		// Tail
		Vertex_PCU(Vec2(-2.f, 0.f), tailColor),
		Vertex_PCU(Vec2(0.f, -0.5f), sideColor),
		Vertex_PCU(Vec2(0.f, 0.5f), sideColor)
	};

	constexpr int NUM_VERTEXES = sizeof(bulletVertexes) / sizeof(bulletVertexes[0]);
	Vertex_PCU::TransformVertexArray(bulletVertexes, NUM_VERTEXES, 1.f, m_orientationDegrees, m_position);

	g_renderer->DrawVertexArray(NUM_VERTEXES, bulletVertexes);
}


//-----------------------------------------------------------------------------------------------
void StarburstBullet::Die()
{
	m_isGarbage = true;

	if ( !IsOffScreen() )
	{
		constexpr float deltaDegrees = 360.f / (float)NUM_STARBURST_BULLETS;
		float curAngleDegrees = 0;
		for ( int bulletIndex = 0; bulletIndex < NUM_STARBURST_BULLETS; bulletIndex++ )
		{
			Vec2 newOrientation = Vec2::MakeFromPolarDegrees(curAngleDegrees);
			m_game->SpawnBullet(m_position + ( newOrientation * 5.f ), newOrientation);
			curAngleDegrees += deltaDegrees;
		}
	}

	Entity::Die();
}

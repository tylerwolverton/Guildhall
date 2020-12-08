#include "Game/Bullet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
Bullet::Bullet( Map* map, EntityType type, EntityFaction faction, const Vec2& position, float orientationDegrees )
	: Entity( map, type, faction, position )
{
	m_orientationDegrees = orientationDegrees;
	m_velocity = Vec2::MakeFromPolarDegrees( m_orientationDegrees ) * BULLET_SPEED;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;

	m_pushesEntities = false;
	m_isPushedByEntities = false;
	m_isPushedByWalls = false;
	m_isHitByBullets = true;

	PopulateVertexes();
}


//-----------------------------------------------------------------------------------------------
Bullet::~Bullet()
{
}


//-----------------------------------------------------------------------------------------------
void Bullet::Update( float deltaSeconds )
{
	m_position += m_velocity * deltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void Bullet::Render() const
{
	std::vector<Vertex_PCU> vertexesCopy( m_vertexes );
	Vertex_PCU::TransformVertexArray( vertexesCopy, BULLET_SPRITE_SCALE, m_orientationDegrees, m_position );

	g_renderer->BindTexture( 0, m_texture );
	g_renderer->DrawVertexArray( vertexesCopy );
}


//-----------------------------------------------------------------------------------------------
void Bullet::Die()
{
	m_isGarbage = true;
	m_isDead = true;

	m_map->SpawnExplosion( m_position, BULLET_COSMETIC_RADIUS * 2.f, .2f );
}


//-----------------------------------------------------------------------------------------------
void Bullet::PopulateVertexes()
{
	switch( m_type )
	{
		case ENTITY_TYPE_GOOD_BULLET:
		{
			m_texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/FriendlyBullet.png" );
		}
		break;

		case ENTITY_TYPE_EVIL_BULLET:
		{
			m_texture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/EnemyBullet.png" );
		}
		break;

		default:
			ERROR_AND_DIE("Invalid bullet type created!");
			break;
	}

	AABB2 spriteBounds( AABB2::ONE_BY_ONE );
	spriteBounds.maxs.x *= 2.f;
	AppendVertsForAABB2D( m_vertexes, spriteBounds, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
}

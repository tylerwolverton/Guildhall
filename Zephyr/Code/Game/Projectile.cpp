#include "Game/Projectile.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Projectile::Projectile( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_canBePushedByWalls = true;
	m_canBePushedByEntities = false;
	m_canPushEntities = false;
	m_willDieOnCollision = true;

	m_damage = entityDef.GetDamageRange().GetRandomInRange( g_game->m_rng );
}


//-----------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}


//-----------------------------------------------------------------------------------------------
void Projectile::Update( float deltaSeconds )
{
	m_velocity = GetForwardVector() * m_entityDef.GetSpeed();

	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Projectile::Render() const
{
	Entity::Render();
}


//-----------------------------------------------------------------------------------------------
void Projectile::Die()
{
	Entity::Die();
}

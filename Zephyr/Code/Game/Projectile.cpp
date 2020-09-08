#include "Game/Projectile.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Projectile::Projectile( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_damage = entityDef.GetDamageRange().GetRandomInRange( g_game->m_rng );

	m_rigidbody2D->SetDrag( 0.f );
	m_rigidbody2D->GetCollider()->m_onOverlapEnterDelegate.SubscribeMethod( this, &Projectile::EnterCollisionEvent );
}


//-----------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}


//-----------------------------------------------------------------------------------------------
void Projectile::Update( float deltaSeconds )
{
	m_rigidbody2D->SetVelocity( GetForwardVector() * m_entityDef.GetSpeed() );
	//m_velocity = GetForwardVector() * m_entityDef.GetSpeed();

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


//-----------------------------------------------------------------------------------------------
void Projectile::EnterCollisionEvent( Collision2D collision )
{
	if ( !IsDead() )
	{
		Entity* theirEntity = (Entity*)collision.theirCollider->m_rigidbody->m_userProperties.GetValue( "entity", ( void* )nullptr );
		
		if ( theirEntity != nullptr )
		{
			theirEntity->TakeDamage( (int)m_damage );
		}

		Die();
	}
}


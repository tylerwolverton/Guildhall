#include "Game/Projectile.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Scripting/ZephyrScript.hpp"


//-----------------------------------------------------------------------------------------------
Projectile::Projectile( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_damage = entityDef.GetDamageRange().GetRandomInRange( g_game->m_rng );

	m_rigidbody2D->SetDrag( 0.f );
	m_rigidbody2D->SetLayer( eCollisionLayer::ENEMY_PROJECTILE );

	if ( m_scriptObj != nullptr )
	{
		ZephyrValueMap globalValues;
		globalValues["attackDamage"] = m_damage;
		m_scriptObj->InitializeGlobalVariables( globalValues );
	}
}


//-----------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}


//-----------------------------------------------------------------------------------------------
void Projectile::Load()
{
	Entity::Load();
	m_rigidbody2D->GetCollider()->m_onOverlapEnterDelegate.SubscribeMethod( this, &Projectile::EnterCollisionEvent );
}


//-----------------------------------------------------------------------------------------------
void Projectile::Update( float deltaSeconds )
{
	m_rigidbody2D->SetVelocity( GetForwardVector() * m_entityDef.GetSpeed() );

	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Projectile::EnterCollisionEvent( Collision2D collision )
{
	if ( !IsDead() )
	{
		Entity* theirEntity = (Entity*)collision.theirCollider->m_rigidbody->m_userProperties.GetValue( "entity", ( void* )nullptr );

		if ( m_scriptObj != nullptr )
		{
			EventArgs args;
			if ( theirEntity != nullptr )
			{
				//theirEntity->TakeDamage( (int)m_damage );
				args.SetValue( "otherId", theirEntity->GetId() );
			}

			m_scriptObj->FireEvent( "Collision", &args );
		}

		//Die();
	}
}


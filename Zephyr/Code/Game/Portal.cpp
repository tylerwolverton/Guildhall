#include "Game/Portal.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
Portal::Portal( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_rigidbody2D->SetSimulationMode( SIMULATION_MODE_STATIC );
	m_rigidbody2D->SetLayer( eCollisionLayer::PORTAL );
}


//-----------------------------------------------------------------------------------------------
Portal::~Portal()
{
}


//-----------------------------------------------------------------------------------------------
void Portal::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Portal::Render() const
{
	Entity::Render();
}


//-----------------------------------------------------------------------------------------------
void Portal::Die()
{
	Entity::Die();
}


//-----------------------------------------------------------------------------------------------
void Portal::Load()
{
	m_rigidbody2D->Enable();

	DiscCollider2D* discCollider = g_physicsSystem2D->CreateDiscTrigger( Vec2::ZERO, GetPhysicsRadius() );
	m_rigidbody2D->TakeCollider( discCollider );

	m_rigidbody2D->GetCollider()->m_onTriggerEnterDelegate.SubscribeMethod( this, &Portal::EnterTriggerEvent );
}


//-----------------------------------------------------------------------------------------------
void Portal::EnterTriggerEvent( Collision2D collision )
{
	if ( !IsDead() )
	{
		Entity* theirObject = (Entity*)collision.theirCollider->m_rigidbody->m_userProperties.GetValue( "entity", ( void* )nullptr );
		if ( theirObject != nullptr )
		{
			m_map->WarpEntityInMap( theirObject, this );
		}
	}
}


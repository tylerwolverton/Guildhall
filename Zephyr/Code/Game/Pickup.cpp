#include "Game/Pickup.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
Pickup::Pickup( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_rigidbody2D->SetLayer( eCollisionLayer::PICKUP );

	m_rigidbody2D->GetCollider()->m_onTriggerEnterDelegate.SubscribeMethod( this, &Pickup::EnterCollisionEvent );
}


//-----------------------------------------------------------------------------------------------
Pickup::~Pickup()
{
}


//-----------------------------------------------------------------------------------------------
void Pickup::EnterCollisionEvent( Collision2D collision )
{
	if ( !IsDead() )
	{
		Entity* theirObject = (Entity*)collision.theirCollider->m_rigidbody->m_userProperties.GetValue( "entity", ( void* )nullptr );
		if ( theirObject != nullptr )
		{
			m_map->AddItemToTargetInventory( this, theirObject );
		}
	}
}


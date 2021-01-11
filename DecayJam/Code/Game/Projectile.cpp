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
void Projectile::Update( float deltaSeconds )
{
	m_rigidbody2D->SetVelocity( GetForwardVector() * m_entityDef.GetSpeed() );

	Entity::Update( deltaSeconds );
}

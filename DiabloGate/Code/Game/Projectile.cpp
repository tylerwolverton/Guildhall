#include "Game/Projectile.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Projectile::Projectile( const EntityDefinition& entityDef, Map* curMap )
	: Entity( entityDef, curMap )
{
	m_rigidbody2D->SetDrag( 0.f );
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


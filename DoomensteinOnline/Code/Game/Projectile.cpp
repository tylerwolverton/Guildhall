#include "Game/Projectile.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Projectile::Projectile( const EntityDefinition& entityDef )
	: Entity( entityDef )
{

}


//-----------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}


//-----------------------------------------------------------------------------------------------
void Projectile::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Projectile::Render() const
{

}


//-----------------------------------------------------------------------------------------------
void Projectile::Die()
{
	Entity::Die();
}

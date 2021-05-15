#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class EntityDefinition;


//-----------------------------------------------------------------------------------------------
class Projectile : public Entity
{
public:
	Projectile( const EntityDefinition& entityDef, Map* curMap );
	~Projectile();

	virtual void Update( float deltaSeconds );
};

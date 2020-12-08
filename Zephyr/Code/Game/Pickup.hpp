#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class EntityDefinition;
class Collision2D;


//-----------------------------------------------------------------------------------------------
class Pickup : public Entity
{
public:
	Pickup( const EntityDefinition& entityDef, Map* map );
	~Pickup();

	virtual void Load() override;

private:
	void EnterCollisionEvent( Collision2D collision );
};

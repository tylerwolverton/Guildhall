#pragma once
#include "Game/Entity.hpp"

//-----------------------------------------------------------------------------------------------
class EntityDefinition;


//-----------------------------------------------------------------------------------------------
class Portal : public Entity
{
public:
	Portal( const EntityDefinition& entityDef );
	~Portal();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();
};

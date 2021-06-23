#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class EntityDefinition;


//-----------------------------------------------------------------------------------------------
class Projectile : public Entity
{
public:
	Projectile( const EntityDefinition& entityDef, Map* map );
	virtual ~Projectile();
	
	virtual void Update( float deltaSeconds ) override;
	
private:
	float m_damage = 0.f;
};

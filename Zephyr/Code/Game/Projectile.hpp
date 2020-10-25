#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class EntityDefinition;
class Collision2D;


//-----------------------------------------------------------------------------------------------
class Projectile : public Entity
{
public:
	Projectile( const EntityDefinition& entityDef, Map* map );
	~Projectile();

	virtual void Load() override;

	virtual void Update( float deltaSeconds ) override;

private:
	void EnterCollisionEvent( Collision2D collision );

private:
	float m_damage = 0.f;
};

#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class EntityDefinition;


//-----------------------------------------------------------------------------------------------
class Projectile : public Entity
{
public:
	Projectile( const EntityDefinition& entityDef, Map* map );
	~Projectile();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

private:
	float m_damage = 0.f;
};

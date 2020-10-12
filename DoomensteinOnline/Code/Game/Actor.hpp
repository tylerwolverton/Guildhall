#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;
class EntityDefinition;
class SpriteAnimDefinition;


//-----------------------------------------------------------------------------------------------
class Actor : public Entity
{
public:
	Actor( const EntityDefinition& entityDef );
	virtual ~Actor() {}

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();

protected:
	int						m_controllerID = -1;
	float					m_cumulativeTime = 0.f;
	
	SpriteAnimDefinition*	m_curAnimDef = nullptr;
};
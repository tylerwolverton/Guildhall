#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class EntityDefinition;
class SpriteAnimDefinition;



//-----------------------------------------------------------------------------------------------
class Cursor : public Entity
{
public:
	Cursor( const Vec2& position, EntityDefinition* entityDef );
	~Cursor();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void Die();
	
protected:
	SpriteAnimDefinition* m_curAnimDef = nullptr;
	float				  m_cumulativeTime = 0.f;
};

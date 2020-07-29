#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class EntityDefinition;
class Actor;


//-----------------------------------------------------------------------------------------------
class Portal : public Entity
{
public:
	Portal( const Vec2& position, EntityDefinition* itemDef );
	~Portal();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	void OnEnter( Actor* actor );

	bool IsActive() const							{ return m_isActive; }
	void Activate()									{ m_isActive = true; }
	void Deactivate()								{ m_isActive = false; }

protected:
	bool					m_isActive = true;
};

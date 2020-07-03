#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class PortalDefinition;
class Actor;


//-----------------------------------------------------------------------------------------------
class Portal : public Entity
{
public:
	Portal( const Vec2& position, PortalDefinition* itemDef );
	~Portal();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;

	void OnEnter( Actor* actor );

protected:
	PortalDefinition*		m_portalDef = nullptr;
};

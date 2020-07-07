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

	bool IsActive() const							{ return m_isActive; }
	void Activate()									{ m_isActive = true; }
	void Deactivate()								{ m_isActive = false; }

protected:
	PortalDefinition*		m_portalDef = nullptr;
	bool					m_isActive = true;
};

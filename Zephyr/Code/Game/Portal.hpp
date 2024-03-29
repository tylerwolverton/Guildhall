#pragma once
#include "Game/Entity.hpp"

//-----------------------------------------------------------------------------------------------
class EntityDefinition;
class Collision2D;


//-----------------------------------------------------------------------------------------------
class Portal : public Entity
{
public:
	Portal( const EntityDefinition& entityDef, Map* map );
	virtual ~Portal();

	//virtual void Load() override;

	std::string GetDestinationMap() const							{ return m_destMap; }
	void SetDestinationMap( const std::string& destMap )			{ m_destMap = destMap; }
	Vec2 GetDestinationPosition()									{ return m_destPos; }
	void SetDestinationPosition( const Vec2& destPos )				{ m_destPos = destPos; }
	float GetDestinationYawOffset()									{ return m_destYawOffset; }
	void SetDestinationYawOffset( float destYawOffset )				{ m_destYawOffset = destYawOffset; }

private:
	void EnterTriggerEvent( Collision2D collision );

private:
	std::string m_destMap = "";
	Vec2		m_destPos = Vec2::ZERO;
	float		m_destYawOffset = 0.f;
};

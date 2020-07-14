#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;
class SpriteAnimDefinition;
class SpriteAnimSetDefinition;


//-----------------------------------------------------------------------------------------------
class EntityDefinition
{
	friend class Entity;
	friend class Cursor;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem );
	~EntityDefinition();

	std::string GetName()											{ return m_name; }
	std::string GetType()											{ return m_type; }
	void SetType( const std::string& type )							{ m_type = type; }
	std::string GetFaction()										{ return m_faction; }
	SpriteAnimDefinition* GetSpriteAnimDef( const std::string& animName );

	bool CanWalk()													{ return m_canWalk; }
	bool CanFly()													{ return m_canFly; }
	bool CanSwim()													{ return m_canSwim; }
	
	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	std::string		m_name;
	std::string		m_type;
	std::string		m_faction;
	float			m_physicsRadius = 0.f;
	AABB2			m_localDrawBounds = AABB2::ONE_BY_ONE;
	int			    m_drawOrder = 0;
	int				m_maxHealth = 1;
	int				m_startHealth = 1;
	SpriteSheet*	m_spriteSheet = nullptr;
	SpriteAnimSetDefinition* m_spriteAnimSetDef = nullptr;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;

	bool			m_canWalk = false;
	bool			m_canFly = false;
	bool			m_canSwim = false;

};

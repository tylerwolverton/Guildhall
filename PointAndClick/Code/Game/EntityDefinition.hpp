#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class NamedProperties;
class SpriteSheet;
class SpriteAnimDefinition;
class SpriteAnimSetDefinition;
enum class eVerbState : int;


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
	
	std::string GetDestinationMapName()								{ return m_destinationMapName; }

	NamedProperties* GetVerbEventProperties( eVerbState verbState );
	SpriteAnimDefinition* GetSpriteAnimDef( const std::string& animName );
		
	static EntityDefinition* GetEntityDefinition( std::string entityName );

private:
	void ParseActionEventsFromXml( const XmlElement& entityDefElem );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	std::string		m_name;
	std::string		m_type;
	float			m_walkSpeed = 0.f;
	float			m_physicsRadius = 0.f;
	AABB2			m_localDrawBounds = AABB2::ONE_BY_ONE;
	int			    m_drawOrder = 0;
	int				m_maxHealth = 1;
	int				m_startHealth = 1;
	SpriteSheet*	m_spriteSheet = nullptr;
	SpriteAnimSetDefinition* m_spriteAnimSetDef = nullptr;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;
	
	// Portal variables
	std::string		m_destinationMapName;

private:
	std::map<eVerbState, NamedProperties*> m_verbPropertiesMap;
};

#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;


//-----------------------------------------------------------------------------------------------
class EntityDefinition
{
	friend class Entity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem );

	bool		IsValid()											{ return m_isValid; }
	std::string GetName()											{ return m_name; }
	float		GetWalkSpeed()										{ return m_walkSpeed; }

	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_name;
	float			m_physicsRadius = 0.f;
	float			m_physicsHeight = 0.f;
	float			m_mass = 1.f;
	float			m_walkSpeed = 0.f;
};

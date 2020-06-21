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

	bool		IsValid() const											{ return m_isValid; }
	std::string GetName() const											{ return m_name; }
	std::string GetType() const											{ return m_type; }
	float		GetWalkSpeed() const									{ return m_walkSpeed; }

	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_name;
	std::string		m_type;
	float			m_physicsRadius = 0.f;
	float			m_physicsHeight = 0.f;
	float			m_mass = 1.f;
	float			m_walkSpeed = 0.f;
};

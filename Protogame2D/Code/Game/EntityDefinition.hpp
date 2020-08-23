#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;
class SpriteAnimationSetDefinition;


//-----------------------------------------------------------------------------------------------
enum eEntityType
{
	UNKNOWN = -1,
	ENTITY,
	ACTOR,
	PROJECTILE,
	PORTAL,
};

std::string GetEntityTypeAsString( eEntityType entityType );


//-----------------------------------------------------------------------------------------------
class EntityDefinition
{
	friend class Entity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem, SpriteSheet* spritSheet );
	~EntityDefinition();

	bool		IsValid() const																{ return m_isValid; }
	std::string GetName() const																{ return m_name; }
	eEntityType GetType() const																{ return m_type; }
	float		GetWalkSpeed() const														{ return m_walkSpeed; }
	std::map< std::string, SpriteAnimationSetDefinition* > GetSpriteAnimSetDefs() const		{ return m_spriteAnimSetDefs; }
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;

	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_name;
	eEntityType		m_type = eEntityType::UNKNOWN;
	float			m_physicsRadius = 0.f;
	float			m_mass = 1.f;
	float			m_walkSpeed = 0.f;

	AABB2			m_localDrawBounds;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;

	std::map< std::string, SpriteAnimationSetDefinition* > m_spriteAnimSetDefs;
};

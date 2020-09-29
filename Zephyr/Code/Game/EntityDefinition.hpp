#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Game/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;
class SpriteAnimationSetDefinition;
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
enum class eEntityType
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

	bool			IsValid() const																{ return m_isValid; }
	std::string		GetName() const																{ return m_name; }
	int				GetMaxHealth() const														{ return m_maxHealth; }
	eEntityType		GetType() const																{ return m_type; }
	
	float			GetWalkSpeed() const														{ return m_walkSpeed; }
	float			GetSpeed() const															{ return m_speed; }
	eCollisionLayer	GetCollisionLayer() const													{ return m_collisionLayer; }

	FloatRange		GetDamageRange() const														{ return m_damageRange; }
	
	std::map< std::string, SpriteAnimationSetDefinition* > GetSpriteAnimSetDefs() const		{ return m_spriteAnimSetDefs; }
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;

	std::map<std::string, std::string> GetRegisteredEvents() const							{ return m_receivedEventsToResponseEvents; }

	ZephyrScriptDefinition* GetZephyrScriptDefinition() const								{ return m_zephyrScriptDef; }

	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_name;
	eEntityType		m_type = eEntityType::UNKNOWN;
	eCollisionLayer	m_collisionLayer = eCollisionLayer::NONE;
	int				m_maxHealth = 1;
	float			m_physicsRadius = 0.f;
	float			m_mass = 1.f;
	float			m_walkSpeed = 0.f;
	float			m_speed = 0.f;

	FloatRange		m_damageRange;

	AABB2			m_localDrawBounds;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;

	// Events
	std::string		m_birthEventName;
	std::string		m_deathEventName;
	std::map<std::string, std::string> m_receivedEventsToResponseEvents;

	// Scripts
	ZephyrScriptDefinition* m_zephyrScriptDef = nullptr;

	std::map< std::string, SpriteAnimationSetDefinition* > m_spriteAnimSetDefs;
};

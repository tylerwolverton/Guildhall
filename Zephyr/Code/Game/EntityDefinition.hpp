#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"
#include "Engine/ZephyrCore/ZephyrEntityDefinition.hpp"
#include "Game/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;
class SpriteAnimationSetDefinition;
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
enum class eEntityClass
{
	UNKNOWN = -1,
	ENTITY,
	ACTOR,
	PROJECTILE,
	PORTAL,
	PICKUP,
};

std::string GetEntityClassAsString( eEntityClass entityClass );


//-----------------------------------------------------------------------------------------------
class EntityDefinition : public ZephyrEntityDefinition
{
	friend class Entity;
	friend class Actor;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem, SpriteSheet* spriteSheet );
	virtual ~EntityDefinition();
	
	bool			IsValid() const																{ return m_isValid; }
	std::string		GetType() const																{ return m_type; }
	float			GetMaxHealth() const														{ return m_maxHealth; }
	eEntityClass	GetClass() const															{ return m_class; }
	
	float			GetSpeed() const															{ return m_speed; }
	float			GetMass() const																{ return m_mass; }
	float			GetDrag() const																{ return m_drag; }
	eCollisionLayer	GetCollisionLayer() const													{ return m_collisionLayer; }
	eSimulationMode	GetSimMode() const															{ return m_simMode; }
	bool			IsTrigger() const															{ return m_isTrigger; }

	FloatRange		GetDamageRange() const														{ return m_damageRange; }

	SpriteAnimationSetDefinition* GetDefaultSpriteAnimSetDef() const							{ return m_defaultSpriteAnimSetDef; }
	std::map< std::string, SpriteAnimationSetDefinition* > GetSpriteAnimSetDefs() const			{ return m_spriteAnimSetDefs; }
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;
	
	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_type;
	eEntityClass	m_class = eEntityClass::UNKNOWN;
	eCollisionLayer	m_collisionLayer = eCollisionLayer::NONE;
	eSimulationMode	m_simMode = eSimulationMode::SIMULATION_MODE_NONE;
	bool			m_isTrigger = false;
	float			m_maxHealth = 1.f;
	float			m_physicsRadius = 0.f;
	float			m_mass = 10.f;
	float			m_drag = 0.f;
	float			m_speed = 0.f;

	FloatRange		m_damageRange = FloatRange( 0.f );

	AABB2			m_localDrawBounds;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;
	
	std::map< std::string, SpriteAnimationSetDefinition* > m_spriteAnimSetDefs;
	SpriteAnimationSetDefinition* m_defaultSpriteAnimSetDef = nullptr;
};

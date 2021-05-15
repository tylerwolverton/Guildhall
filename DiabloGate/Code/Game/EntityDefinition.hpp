#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Scripting/ZephyrCommon.hpp"

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
	ACTOR_SPAWNER,
	PROJECTILE,
	PORTAL,
	ITEM
};

std::string GetEntityClassAsString( eEntityClass entityClass );
eEntityClass GetEntityClassFromString( const std::string& entityClass );


//-----------------------------------------------------------------------------------------------
enum class eItemType
{
	NONE,
	ANY,
	WEAPON,
	SHIELD,
	ARMOR,
	RING
};

std::string GetItemTypeAsString( eItemType itemType );
eItemType GetItemTypeFromString( const std::string& itemType );


//-----------------------------------------------------------------------------------------------
struct LootDrop
{
	std::string entityDefName;
	float dropRate;
};


//-----------------------------------------------------------------------------------------------
struct SpawnTarget
{
	std::string entityDefName;
	float spawnChance = 1.f;
};


//-----------------------------------------------------------------------------------------------
class EntityDefinition
{
	friend class Entity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem, SpriteSheet* defaultSpriteSheet );
	~EntityDefinition();

	void						ReloadZephyrScriptDefinition();

	bool						IsValid() const																{ return m_isValid; }
	std::string					GetName() const																{ return m_name; }
	std::string					GetType() const																{ return m_type; }
	//float						GetMaxHealth() const														{ return m_maxHealth; }
	eEntityClass				GetClass() const															{ return m_class; }
	eItemType					GetItemType() const															{ return m_itemType; }

	float						GetSpeed() const															{ return m_speed; }
	float						GetMass() const																{ return m_mass; }
	float						GetDrag() const																{ return m_drag; }
	eCollisionLayer				GetCollisionLayer() const													{ return m_collisionLayer; }
	eSimulationMode				GetSimMode() const															{ return m_simMode; }
	bool						IsTrigger() const															{ return m_isTrigger; }

	float						GetInteractionRadius() const												{ return m_interactionRadius; }
	Vec2						GetInteractionRadiusOffset() const											{ return m_interactionRadiusOffset; }
	int							GetInitialMaxHealth() const													{ return m_initMaxHealth; }
	IntRange					GetInitialAttackDamageRange() const											{ return m_initAttackDamageRange; }
	IntRange					GetInitialDefenseRange() const												{ return m_initDefenseRange; }
	float						GetInitialAttackRange() const												{ return m_initAttackRange; }
	float						GetInitialAttackSpeedMultiplier() const										{ return m_initAttackSpeedModifier; }
	float						GetInitialCritChance() const												{ return m_initCritChance; }
	int							GetXPReward() const															{ return m_xpReward; }
	std::vector<LootDrop>		GetLootDrops() const														{ return m_lootDrops; }
	float						GetSpawnCooldown() const													{ return m_spawnCooldown; }
	int							GetMaxSpawnWaves() const													{ return m_maxSpawnWaves; }
	std::vector<SpawnTarget>	GetSpawnTargets() const														{ return m_spawnTargets; }

	AABB2						GetLocalDrawBounds() const													{ return m_localDrawBounds; }

	SpriteAnimationSetDefinition* GetDefaultSpriteAnimSetDef() const										{ return m_defaultSpriteAnimSetDef; }
	std::map< std::string, SpriteAnimationSetDefinition* > GetSpriteAnimSetDefs() const						{ return m_spriteAnimSetDefs; }
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;

	ZephyrScriptDefinition* GetZephyrScriptDefinition() const												{ return m_zephyrScriptDef; }
	ZephyrValueMap GetZephyrScriptInitialValues() const														{ return m_zephyrScriptInitialValues; }
	std::vector<EntityVariableInitializer> GetZephyrEntityVarInits() const									{ return m_zephyrEntityVarInits; }

	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_name;
	std::string		m_type;
	eItemType		m_itemType = eItemType::NONE;
	eEntityClass	m_class = eEntityClass::UNKNOWN;
	float			m_interactionRadius = 1.f;
	Vec2			m_interactionRadiusOffset = Vec2::ZERO;

	eCollisionLayer	m_collisionLayer = eCollisionLayer::NONE;
	eSimulationMode	m_simMode = eSimulationMode::SIMULATION_MODE_NONE;
	bool			m_isTrigger = false;
	float			m_physicsRadius = 0.f;
	float			m_mass = 1.f;
	float			m_drag = 0.f;
	float			m_speed = 0.f;

	// Stats
	int				m_initMaxHealth = 9999999;
	IntRange		m_initAttackDamageRange;
	IntRange		m_initDefenseRange;
	float			m_initAttackRange = 0.f;
	float			m_initAttackSpeedModifier = 0.f;
	float			m_initCritChance = 0;
	int				m_xpReward = 0;

	std::vector<LootDrop> m_lootDrops;
	std::vector<SpawnTarget> m_spawnTargets;

	float m_spawnCooldown = 1.f;
	int m_maxSpawnWaves = 1;

	// Scripts
	std::string								m_zephyrScriptName;
	ZephyrScriptDefinition*					m_zephyrScriptDef = nullptr;
	ZephyrValueMap							m_zephyrScriptInitialValues;
	std::vector<EntityVariableInitializer>  m_zephyrEntityVarInits;

	// Visual
	AABB2			m_localDrawBounds;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;

	std::map< std::string, SpriteAnimationSetDefinition* > m_spriteAnimSetDefs;
	SpriteAnimationSetDefinition* m_defaultSpriteAnimSetDef = nullptr;
};

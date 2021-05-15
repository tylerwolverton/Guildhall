#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/Scripting/GameAPI.hpp"
#include "Game/Scripting/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, EntityDefinition* > EntityDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition* EntityDefinition::GetSpriteAnimSetDef( const std::string& animSetName ) const
{
	auto mapIter = m_spriteAnimSetDefs.find( animSetName );

	if ( mapIter == m_spriteAnimSetDefs.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition* EntityDefinition::GetEntityDefinition( std::string entityName )
{
	std::map< std::string, EntityDefinition* >::const_iterator  mapIter = EntityDefinition::s_definitions.find( entityName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::EntityDefinition( const XmlElement& entityDefElem, SpriteSheet* defaultSpriteSheet )
{
	m_name = ParseXmlAttribute( entityDefElem, "name", "" );
	if ( m_name == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: EntityType is missing a name attribute" );
		return;
	}
	
	std::string typeStr = entityDefElem.Name();
	if ( typeStr == "Entity" )
	{
		m_class = eEntityClass::ENTITY;
	}
	else if ( typeStr == "Actor" )
	{
		m_class = eEntityClass::ACTOR;
	}
	else if ( typeStr == "Projectile" )
	{
		m_class = eEntityClass::PROJECTILE;
	}
	else if ( typeStr == "Portal" )
	{
		m_class = eEntityClass::PORTAL;
	}
	else if ( typeStr == "ActorSpawner" )
	{
		m_class = eEntityClass::ACTOR_SPAWNER;
		m_spawnCooldown = ParseXmlAttribute( entityDefElem, "spawnCooldown", m_spawnCooldown );
		m_maxSpawnWaves = ParseXmlAttribute( entityDefElem, "maxSpawnWaves", m_maxSpawnWaves );
	}
	else if ( typeStr == "Item" )
	{
		m_class = eEntityClass::ITEM;
		std::string itemTypeStr = ParseXmlAttribute( entityDefElem, "itemType", "" );
		m_itemType = GetItemTypeFromString( itemTypeStr );
	}
	else
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unsupported entity type seen, '%s'", typeStr.c_str() ) );
		return;
	}

	// Physics
	const XmlElement* physicsElem = entityDefElem.FirstChildElement( "Physics" );
	if ( physicsElem != nullptr )
	{
		m_physicsRadius = ParseXmlAttribute( *physicsElem, "radius", m_physicsRadius );
		m_mass = ParseXmlAttribute( *physicsElem, "mass", m_mass );

		m_drag = ParseXmlAttribute( *physicsElem, "drag", m_drag );
		m_speed = ParseXmlAttribute( *physicsElem, "speed", m_speed );

		std::string simModeStr = ParseXmlAttribute( *physicsElem, "simMode", "" );

		// optional param, if empty just use layer's mode
		if ( !simModeStr.empty() )
		{
			if ( IsEqualIgnoreCase( simModeStr, "static" ) ) { m_simMode = eSimulationMode::SIMULATION_MODE_STATIC; }
			else if ( IsEqualIgnoreCase( simModeStr, "kinematic" ) ) { m_simMode = eSimulationMode::SIMULATION_MODE_KINEMATIC; }
			else if ( IsEqualIgnoreCase( simModeStr, "dynamic" ) ) { m_simMode = eSimulationMode::SIMULATION_MODE_DYNAMIC; }
			else
			{
				g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unsupported simMode attribute, '%s', can be static, kinematic, or dynamic", simModeStr.c_str() ) );
				return;
			}
		}

		std::string collisionLayerStr = ParseXmlAttribute( *physicsElem, "collisionLayer", "" );
		m_collisionLayer = GetCollisionLayerFromString( collisionLayerStr );

		m_isTrigger = ParseXmlAttribute( *physicsElem, "isTrigger", false );
	}

	// Appearance
	const XmlElement* appearanceElem = entityDefElem.FirstChildElement( "Appearance" );
	if ( appearanceElem != nullptr )
	{
		SpriteSheet* spriteSheet = defaultSpriteSheet;

		// Parse sprite sheet if defined
		std::string spriteSheetPath = ParseXmlAttribute( *appearanceElem, "spriteSheet", "" );
		if ( spriteSheetPath != "" )
		{
			spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );

			// This is a new spritesheet, register it
			if ( spriteSheet == nullptr )
			{
				IntVec2 spriteSheetDimensions = ParseXmlAttribute( *appearanceElem, "spriteSheetDimensions", IntVec2( -1, -1 ) );
				if ( spriteSheetDimensions == IntVec2( -1, -1 ) )
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml: Missing spriteSheetDimensions attribute for '%s'", m_name.c_str() ) );
					return;
				}

				Texture* texture = g_renderer->CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
				if ( texture == nullptr )
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml: Couldn't load texture '%s'", spriteSheetPath.c_str() ) );
					return;
				}

				spriteSheet = SpriteSheet::CreateAndRegister( *texture, spriteSheetDimensions );
			}
		}

		m_localDrawBounds = ParseXmlAttribute( *appearanceElem, "localDrawBounds", m_localDrawBounds );
		float defaultFPS = ParseXmlAttribute( *appearanceElem, "fps", 1.f );

		bool isFirstAnim = true;

		const XmlElement* animationSetElem = appearanceElem->FirstChildElement();
		while ( animationSetElem != nullptr )
		{
			m_spriteAnimSetDefs[animationSetElem->Name()] = new SpriteAnimationSetDefinition( spriteSheet, *animationSetElem, defaultFPS );

			if ( isFirstAnim )
			{
				isFirstAnim = false;
				m_defaultSpriteAnimSetDef = m_spriteAnimSetDefs[animationSetElem->Name()];
			}

			animationSetElem = animationSetElem->NextSiblingElement();
		}
	}

	// Gameplay
	const XmlElement* gameplayElem = entityDefElem.FirstChildElement( "Gameplay" );
	if ( gameplayElem != nullptr )
	{
		m_interactionRadius = ParseXmlAttribute( *gameplayElem, "interactionRadius", m_interactionRadius );
		m_interactionRadiusOffset = ParseXmlAttribute( *gameplayElem, "interactionRadiusOffset", m_interactionRadiusOffset );

		const XmlElement* statElem = gameplayElem->FirstChildElement( "Stat" );
		while ( statElem != nullptr )
		{
			std::string statTypeStr = ParseXmlAttribute( *statElem, "type", "" );
			if ( statTypeStr.empty() )
			{
				g_devConsole->PrintError( Stringf( "EntityTypes.xml: Missing type attribute for stat in '%s'", m_name.c_str() ) );
				statElem = statElem->NextSiblingElement( "Stat" );
				continue;
			}

			if ( IsEqualIgnoreCase( statTypeStr, "maxHealth" ) )
			{
				m_initMaxHealth = ParseXmlAttribute( *statElem, "val", m_initMaxHealth );
			}
			else if ( IsEqualIgnoreCase( statTypeStr, "attackDamage" ) )
			{
				m_initAttackDamageRange = ParseXmlAttribute( *statElem, "val", m_initAttackDamageRange );
			}
			else if ( IsEqualIgnoreCase( statTypeStr, "defense" ) )
			{
				m_initDefenseRange = ParseXmlAttribute( *statElem, "val", m_initDefenseRange );
			}
			else if ( IsEqualIgnoreCase( statTypeStr, "attackRange" ) )
			{
				m_initAttackRange = ParseXmlAttribute( *statElem, "val", m_initAttackRange );
			}
			else if ( IsEqualIgnoreCase( statTypeStr, "attackSpeed" ) )
			{
				m_initAttackSpeedModifier = ParseXmlAttribute( *statElem, "val", m_initAttackSpeedModifier );
			}
			else if ( IsEqualIgnoreCase( statTypeStr, "critChance" ) )
			{
				m_initCritChance = ParseXmlAttribute( *statElem, "val", m_initCritChance );
			}
			else if ( IsEqualIgnoreCase( statTypeStr, "xpReward" ) )
			{
				m_xpReward = ParseXmlAttribute( *statElem, "val", m_xpReward );
			}
			else
			{
				g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unrecognized stat type '%s' seen for entity '%s'", statTypeStr.c_str(), m_name.c_str() ) );
			}

			statElem = statElem->NextSiblingElement( "Stat" );
		}

		const XmlElement* lootElem = gameplayElem->FirstChildElement( "Loot" );
		while ( lootElem != nullptr )
		{
			std::string lootTypeStr = ParseXmlAttribute( *lootElem, "type", "" );
			if ( lootTypeStr.empty() )
			{
				g_devConsole->PrintError( Stringf( "EntityTypes.xml: Missing type attribute for loot in '%s'", m_name.c_str() ) );
				lootElem = lootElem->NextSiblingElement( "Loot" );
				continue;
			}

			float dropRate = ParseXmlAttribute( *lootElem, "dropRate", 0.f );

			LootDrop lootDrop;
			lootDrop.entityDefName = lootTypeStr;
			lootDrop.dropRate = dropRate;

			m_lootDrops.push_back( lootDrop );

			lootElem = lootElem->NextSiblingElement( "Loot" );
		}

		const XmlElement* spawnElem = gameplayElem->FirstChildElement( "Spawn" );
		while ( spawnElem != nullptr )
		{
			std::string spawnTypeStr = ParseXmlAttribute( *spawnElem, "type", "" );
			if ( spawnTypeStr.empty() )
			{
				g_devConsole->PrintError( Stringf( "EntityTypes.xml: Missing type attribute for spawn in '%s'", m_name.c_str() ) );
				spawnElem = spawnElem->NextSiblingElement( "Spawn" );
				continue;
			}

			//float dropRate = ParseXmlAttribute( *spawnElem, "dropRate", 0.f );

			SpawnTarget spawnTarget;
			spawnTarget.entityDefName = spawnTypeStr;
			//spawnTarget.dropRate = dropRate;

			m_spawnTargets.push_back( spawnTarget );

			spawnElem = spawnElem->NextSiblingElement( "Spawn" );
		}
	}


	// Script
	const XmlElement* scriptElem = entityDefElem.FirstChildElement( "Script" );
	if ( scriptElem != nullptr )
	{
		m_zephyrScriptName = ParseXmlAttribute( *scriptElem, "name", "" );

		if ( !m_zephyrScriptName.empty() )
		{
			m_zephyrScriptDef = ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( m_zephyrScriptName );

			if ( m_zephyrScriptDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': Script '%s' does not exist", m_type.c_str(), m_zephyrScriptName.c_str() ) );
				return;
			}

			// Parse initial values
			const XmlElement* globalVarElem = scriptElem->FirstChildElement( "GlobalVar" );
			while ( globalVarElem != nullptr )
			{
				std::string typeName = ParseXmlAttribute( *globalVarElem, "type", "" );
				std::string varName = ParseXmlAttribute( *globalVarElem, "var", "" );
				std::string valueStr = ParseXmlAttribute( *globalVarElem, "value", "" );
				if ( typeName.empty() )
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar is missing a variable type", m_type.c_str() ) );
					break;
				}
				if ( varName.empty() )
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar is missing a variable name", m_type.c_str() ) );
					break;
				}
				if ( valueStr.empty() )
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar is missing a variable value", m_type.c_str() ) );
					break;
				}

				if ( varName == PARENT_ENTITY_NAME )
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar cannot initialize reserved entity variable '%s'.", m_type.c_str(), PARENT_ENTITY_NAME.c_str() ) );
					break;
				}

				// Convert value to correct type and store in map
				if ( !_strcmpi( typeName.c_str(), "string" ) )
				{
					m_zephyrScriptInitialValues[varName] = ZephyrValue( valueStr );
				}
				else if ( !_strcmpi( typeName.c_str(), "number" ) )
				{
					m_zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, 0.f ) );
				}
				else if ( !_strcmpi( typeName.c_str(), "bool" ) )
				{
					m_zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, false ) );
				}
				else if ( !_strcmpi( typeName.c_str(), "vec2" ) )
				{
					m_zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, Vec2::ZERO ) );
				}
				else if ( !_strcmpi( typeName.c_str(), "entity" ) )
				{
					m_zephyrEntityVarInits.emplace_back( varName, valueStr );
				}
				else
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar '%s' has unsupported type '%s'", m_type.c_str(), varName.c_str(), typeName.c_str() ) );
					break;
				}

				globalVarElem = globalVarElem->NextSiblingElement( "GlobalVar" );
			}
		}
	}

	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{
	PTR_MAP_SAFE_DELETE( m_spriteAnimSetDefs );
}


//-----------------------------------------------------------------------------------------------
void EntityDefinition::ReloadZephyrScriptDefinition()
{
	if ( !m_zephyrScriptName.empty() )
	{
		m_zephyrScriptDef = ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( m_zephyrScriptName );
	}
}


//-----------------------------------------------------------------------------------------------
std::string GetEntityClassAsString( eEntityClass entityType )
{
	switch ( entityType )
	{
		case eEntityClass::ACTOR: return "Actor";
		case eEntityClass::ACTOR_SPAWNER: return "ActorSpawner";
		case eEntityClass::PROJECTILE: return "Projectile";
		case eEntityClass::PORTAL: return "Portal";
		case eEntityClass::ENTITY: return "Entity";
		case eEntityClass::ITEM: return "Item";
		default: return "Unknown";
	}
}


//-----------------------------------------------------------------------------------------------
eEntityClass GetEntityClassFromString( const std::string& entityType )
{
	if ( IsEqualIgnoreCase( entityType, "Actor" ) )			{ return eEntityClass::ACTOR; }
	if ( IsEqualIgnoreCase( entityType, "ActorSpawner" ) )	{ return eEntityClass::ACTOR_SPAWNER; }
	if ( IsEqualIgnoreCase( entityType, "Projectile" ) )	{ return eEntityClass::PROJECTILE; }
	if ( IsEqualIgnoreCase( entityType, "Portal" ) )		{ return eEntityClass::PORTAL; }
	if ( IsEqualIgnoreCase( entityType, "Entity" ) )		{ return eEntityClass::ENTITY; }
	if ( IsEqualIgnoreCase( entityType, "Item" ) )			{ return eEntityClass::ITEM; }

	return eEntityClass::UNKNOWN;
}


//-----------------------------------------------------------------------------------------------
std::string GetItemTypeAsString( eItemType itemType )
{
	switch ( itemType )
	{
		case eItemType::ANY:	return "Any";
		case eItemType::WEAPON: return "Weapon";
		case eItemType::ARMOR:	return "Armor";
		case eItemType::SHIELD:	return "Shield";
		case eItemType::RING :	return "Ring";
		default: return "";
	}
}


//-----------------------------------------------------------------------------------------------
eItemType GetItemTypeFromString( const std::string& itemType )
{
	if ( IsEqualIgnoreCase( itemType, "Any" ) )		{ return eItemType::ANY; }
	if ( IsEqualIgnoreCase( itemType, "Weapon" ) )	{ return eItemType::WEAPON; }
	if ( IsEqualIgnoreCase( itemType, "Armor" ) )	{ return eItemType::ARMOR; }
	if ( IsEqualIgnoreCase( itemType, "Shield" ) )	{ return eItemType::SHIELD; }
	if ( IsEqualIgnoreCase( itemType, "Ring" ) )	{ return eItemType::RING; }

	return eItemType::NONE;
}

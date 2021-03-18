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
EntityDefinition::EntityDefinition( const XmlElement& entityDefElem, SpriteSheet* spriteSheet )
{
	m_type = ParseXmlAttribute( entityDefElem, "name", "" );
	if ( m_type == "" )
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
	else if ( typeStr == "Pickup" )
	{
		m_class = eEntityClass::PICKUP;
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
		m_maxHealth = ParseXmlAttribute( *gameplayElem, "maxHealth", m_maxHealth );
		m_damageRange = ParseXmlAttribute( *gameplayElem, "damage", m_damageRange );
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
std::string GetEntityClassAsString( eEntityClass entityClass )
{
	switch ( entityClass )
	{
		case eEntityClass::ACTOR: return "Actor";
		case eEntityClass::PROJECTILE: return "Projectile";
		case eEntityClass::PORTAL: return "Portal";
		case eEntityClass::PICKUP: return "Pickup";
		case eEntityClass::ENTITY: return "Entity";
		default: return "Unknown";
	}
}

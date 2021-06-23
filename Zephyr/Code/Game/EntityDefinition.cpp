#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/ZephyrCore/ZephyrScriptDefinition.hpp"

#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/Scripting/ZephyrGameAPI.hpp"


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
	: ZephyrEntityDefinition( entityDefElem )
{
	// Return if the script parsing failed in base class
	if ( !m_isScriptValid )
	{
		return;
	}

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
	
	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{
	PTR_MAP_SAFE_DELETE( m_spriteAnimSetDefs );
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

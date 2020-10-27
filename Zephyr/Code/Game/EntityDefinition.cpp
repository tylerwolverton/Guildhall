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

		switch ( m_class )
		{
			case eEntityClass::ACTOR: m_walkSpeed = ParseXmlAttribute( *physicsElem, "walkSpeed", m_walkSpeed ); break;
			case eEntityClass::PROJECTILE: m_speed = ParseXmlAttribute( *physicsElem, "speed", m_speed ); break;
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

		const XmlElement* animationSetElem = appearanceElem->FirstChildElement();
		while ( animationSetElem != nullptr )
		{
			m_spriteAnimSetDefs[animationSetElem->Name()] = new SpriteAnimationSetDefinition( spriteSheet, *animationSetElem, defaultFPS );

			animationSetElem = animationSetElem->NextSiblingElement();
		}
	}

	// Gameplay
	const XmlElement* gameplayElem = entityDefElem.FirstChildElement( "Gameplay" );
	if ( gameplayElem != nullptr )
	{
		m_maxHealth = ParseXmlAttribute( *gameplayElem, "maxHealth", m_maxHealth );
		m_damageRange = ParseXmlAttribute( *gameplayElem, "damage", m_damageRange );

		const XmlElement* onBirthElem = gameplayElem->FirstChildElement( "OnBirth" );
		if ( onBirthElem != nullptr )
		{
			m_birthEventName = ParseXmlAttribute( *onBirthElem, "fireEvent", m_birthEventName );

			if ( !g_gameAPI->IsMethodRegistered( m_birthEventName ) )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - Birth event '%s' has not been registered", m_type.c_str(), m_birthEventName.c_str() ) );
			}
		}

		const XmlElement* onDeathElem = gameplayElem->FirstChildElement( "OnDeath" );
		if ( onDeathElem != nullptr )
		{
			m_deathEventName = ParseXmlAttribute( *onDeathElem, "fireEvent", m_deathEventName );

			if ( !g_gameAPI->IsMethodRegistered( m_deathEventName ) )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - Death event '%s' has not been registered", m_type.c_str(), m_deathEventName.c_str() ) );
			}
		}

		const XmlElement* onEventReceivedElem = gameplayElem->FirstChildElement( "OnEventReceived" );
		while ( onEventReceivedElem != nullptr )
		{
			std::string receivedEventName = ParseXmlAttribute( *onEventReceivedElem, "eventName", "" );
			if ( receivedEventName.empty() )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - missing eventName attribute in OnEventReceived node", m_type.c_str() ) );

				onEventReceivedElem = onEventReceivedElem->NextSiblingElement( "OnEventReceived" );
				continue;
			}

			std::string fireEventName = ParseXmlAttribute( *onEventReceivedElem, "fireEvent", "" );
			if ( fireEventName.empty() )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - missing fireEvent attribute in OnEventReceived node", m_type.c_str() ) );

				onEventReceivedElem = onEventReceivedElem->NextSiblingElement( "OnEventReceived" );
				continue;
			}

			if ( !g_gameAPI->IsMethodRegistered( fireEventName ) )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - fireEvent '%s' has not been registered", m_type.c_str(), fireEventName.c_str() ) );

				onEventReceivedElem = onEventReceivedElem->NextSiblingElement( "OnEventReceived" );
				continue;
			}

			// We have valid events
			m_receivedEventsToResponseEvents[receivedEventName] = fireEventName;

			onEventReceivedElem = onEventReceivedElem->NextSiblingElement( "OnEventReceived" );
		}
	}

	const XmlElement* scriptElem = entityDefElem.FirstChildElement( "Script" );
	if ( scriptElem != nullptr )
	{
		m_zephyrScriptName = ParseXmlAttribute( *scriptElem, "name", "" );

		if ( !m_zephyrScriptName.empty() )
		{
			m_zephyrScriptDef = ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( m_zephyrScriptName );
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

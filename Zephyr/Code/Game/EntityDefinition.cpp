#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/Scripting/GameAPI.hpp"


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
	m_name = ParseXmlAttribute( entityDefElem, "name", "" );
	if ( m_name == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: EntityType is missing a name attribute" );
		return;
	}

	std::string typeStr = entityDefElem.Name();
	if ( typeStr == "Entity" )
	{
		m_type = eEntityType::ENTITY;
	}
	else if ( typeStr == "Actor" )
	{
		m_type = eEntityType::ACTOR;
	}
	else if ( typeStr == "Projectile" )
	{
		m_type = eEntityType::PROJECTILE;
	}
	else if ( typeStr == "Portal" )
	{
		m_type = eEntityType::PORTAL;
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

		switch ( m_type )
		{
			case eEntityType::ACTOR: m_walkSpeed = ParseXmlAttribute( *physicsElem, "walkSpeed", m_walkSpeed ); break;
			case eEntityType::PROJECTILE: m_speed = ParseXmlAttribute( *physicsElem, "speed", m_speed ); break;
		}
	}

	// Appearance
	const XmlElement* appearanceElem = entityDefElem.FirstChildElement( "Appearance" );
	if ( appearanceElem != nullptr )
	{
		m_localDrawBounds = ParseXmlAttribute( *appearanceElem, "localDrawBounds", m_localDrawBounds );

		const XmlElement* animationSetElem = appearanceElem->FirstChildElement();
		while ( animationSetElem != nullptr )
		{
			m_spriteAnimSetDefs[animationSetElem->Name()] = new SpriteAnimationSetDefinition( spriteSheet, *animationSetElem );

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
				g_devConsole->PrintError( Stringf( "Entity: '%s' - Birth event '%s' has not been registered", m_name.c_str(), m_birthEventName.c_str() ) );
			}
		}

		const XmlElement* onDeathElem = gameplayElem->FirstChildElement( "OnDeath" );
		if ( onDeathElem != nullptr )
		{
			m_deathEventName = ParseXmlAttribute( *onDeathElem, "fireEvent", m_deathEventName );

			if ( !g_gameAPI->IsMethodRegistered( m_deathEventName ) )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - Death event '%s' has not been registered", m_name.c_str(), m_deathEventName.c_str() ) );
			}
		}

		const XmlElement* onEventReceivedElem = gameplayElem->FirstChildElement( "OnEventReceived" );
		while ( onEventReceivedElem != nullptr )
		{
			std::string receivedEventName = ParseXmlAttribute( *onEventReceivedElem, "eventName", "" );
			if ( receivedEventName.empty() )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - missing eventName attribute in OnEventReceived node", m_name.c_str() ) );

				onEventReceivedElem = onEventReceivedElem->NextSiblingElement( "OnEventReceived" );
				continue;
			}

			std::string fireEventName = ParseXmlAttribute( *onEventReceivedElem, "fireEvent", "" );
			if ( fireEventName.empty() )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - missing fireEvent attribute in OnEventReceived node", m_name.c_str() ) );

				onEventReceivedElem = onEventReceivedElem->NextSiblingElement( "OnEventReceived" );
				continue;
			}

			if ( !g_gameAPI->IsMethodRegistered( fireEventName ) )
			{
				g_devConsole->PrintError( Stringf( "Entity: '%s' - fireEvent '%s' has not been registered", m_name.c_str(), fireEventName.c_str() ) );

				onEventReceivedElem = onEventReceivedElem->NextSiblingElement( "OnEventReceived" );
				continue;
			}

			// We have valid events
			m_receivedEventsToResponseEvents[receivedEventName] = fireEventName;

			onEventReceivedElem = onEventReceivedElem->NextSiblingElement( "OnEventReceived" );
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
std::string GetEntityTypeAsString( eEntityType entityType )
{
	switch ( entityType )
	{
		case eEntityType::ACTOR: return "Actor";
		case eEntityType::PROJECTILE: return "Projectile";
		case eEntityType::PORTAL: return "Portal";
		case eEntityType::ENTITY: return "Entity";
		default: return "Unknown";
	}
}

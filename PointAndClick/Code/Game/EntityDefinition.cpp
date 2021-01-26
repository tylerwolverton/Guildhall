#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"

#include "Game/SpriteAnimSetDefinition.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, EntityDefinition* > EntityDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
EntityDefinition::EntityDefinition( const XmlElement& entityDefElem )
{
	m_name = ParseXmlAttribute( entityDefElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Entity did not have a name attribute" );

	m_type = ParseXmlAttribute( entityDefElem, "type", m_type );

	const XmlElement* destinationElem = entityDefElem.FirstChildElement( "Destination" );
	if ( destinationElem != nullptr )
	{
		m_destinationMapName = ParseXmlAttribute( *destinationElem, "mapName", "" );
	}
	
	const XmlElement* sizeElement = entityDefElem.FirstChildElement( "Size" );
	if(sizeElement != nullptr)
	{
		m_physicsRadius = ParseXmlAttribute( *sizeElement, "physicsRadius", m_physicsRadius );
		m_localDrawBounds = ParseXmlAttribute( *sizeElement, "localDrawBounds", m_localDrawBounds );
		m_drawOrder = ParseXmlAttribute( *sizeElement, "drawOrder", m_drawOrder );
	}

	const XmlElement* movementElement = entityDefElem.FirstChildElement( "Movement" );
	if ( movementElement != nullptr )
	{
		const XmlElement* walkElement = movementElement->FirstChildElement( "Walk" );
		if ( walkElement != nullptr )
		{
			m_walkSpeed = ParseXmlAttribute( *walkElement, "speed", m_walkSpeed );
		}
	}

	const XmlElement* healthElement = entityDefElem.FirstChildElement( "Health" );
	if ( healthElement != nullptr )
	{
		m_maxHealth = ParseXmlAttribute( *healthElement, "max", m_maxHealth );
		m_startHealth = ParseXmlAttribute( *healthElement, "start", m_startHealth );
	}

	const XmlElement* spriteAnimSetElement = entityDefElem.FirstChildElement( "SpriteAnimSet" );
	if ( spriteAnimSetElement != nullptr )
	{
		m_spriteAnimSetDef = new SpriteAnimSetDefinition( *g_renderer, *spriteAnimSetElement );
	}

	ParseActionEventsFromXml( entityDefElem );
}


//-----------------------------------------------------------------------------------------------
void EntityDefinition::ParseActionEventsFromXml( const XmlElement& entityDefElem )
{
	const XmlElement* actionEventsElem = entityDefElem.FirstChildElement( "ActionEvents" );
	if ( actionEventsElem == nullptr )
	{
		return;
	}

	const XmlElement* actionEventElem = actionEventsElem->FirstChildElement( "ActionEvent" );
	while ( actionEventElem != nullptr )
	{
		std::string type = ParseXmlAttribute( *actionEventElem, "type", "" );
		if ( type == "" )
		{
			g_devConsole->PrintError( Stringf( "Item '%s': Missing type attribute for ActionEvent node" ) );

			actionEventElem = actionEventsElem->NextSiblingElement();
			continue;
		}

		Texture* texture = nullptr;
		std::string texturePathStr = ParseXmlAttribute( *actionEventElem, "texturePath", "" );
		if ( texturePathStr != "" )
		{
			texture = g_renderer->CreateOrGetTextureFromFile( texturePathStr.c_str() );
		}

		std::string initialDialogueState = ParseXmlAttribute( *actionEventElem, "initialDialogueState", "" );
		std::string requiredItemName = ParseXmlAttribute( *actionEventElem, "requiredItem", "" );
		std::string acceptedItemName = ParseXmlAttribute( *actionEventElem, "acceptedItem", "" );
		std::string receivedItemmName = ParseXmlAttribute( *actionEventElem, "receivedItem", "" );
		std::string text = ParseXmlAttribute( *actionEventElem, "text", "" );
		std::string failText = ParseXmlAttribute( *actionEventElem, "failText", "" );

		NamedProperties* properties = new NamedProperties();

		eVerbState verbState = GetVerbStateFromString( type );
		properties->SetValue( "eventName", GetEventNameForVerbState( verbState ) );
		properties->SetValue( "texture", (void*)texture );
		properties->SetValue( "initialDialogueState", initialDialogueState );
		properties->SetValue( "requiredItem", requiredItemName );
		properties->SetValue( "acceptedItem", acceptedItemName );
		properties->SetValue( "receivedItem", receivedItemmName );
		properties->SetValue( "text", text );
		properties->SetValue( "failText", failText );

		m_verbPropertiesMap[verbState] = properties;

		actionEventElem = actionEventElem->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{
	PTR_MAP_SAFE_DELETE( m_verbPropertiesMap );
	PTR_SAFE_DELETE( m_spriteAnimSetDef );
}


//-----------------------------------------------------------------------------------------------
NamedProperties* EntityDefinition::GetVerbEventProperties( eVerbState verbState )
{
	auto iter = m_verbPropertiesMap.find( verbState );
	if ( iter == m_verbPropertiesMap.end() )
	{
		return nullptr;
	}

	return iter->second;
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* EntityDefinition::GetSpriteAnimDef( const std::string& animName )
{
	if ( m_spriteAnimSetDef == nullptr )
	{
		return nullptr;
	}

	std::map< std::string, SpriteAnimDefinition* >::const_iterator  mapIter = m_spriteAnimSetDef->m_spriteAnimDefMapByName.find( animName );

	if ( mapIter == m_spriteAnimSetDef->m_spriteAnimDefMapByName.cend() )
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

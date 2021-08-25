#include "Game/ItemDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, ItemDefinition* > ItemDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
ItemDefinition::ItemDefinition( const XmlElement& itemDefElem )
	: EntityDefinition( itemDefElem )
{
	const XmlElement* actionEventsElem = itemDefElem.FirstChildElement( "ActionEvents" );
	if(actionEventsElem == nullptr )
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
ItemDefinition::~ItemDefinition()
{
	PTR_MAP_SAFE_DELETE( m_verbPropertiesMap );
}


//-----------------------------------------------------------------------------------------------
NamedProperties* ItemDefinition::GetVerbEventProperties( eVerbState verbState )
{
	auto iter = m_verbPropertiesMap.find( verbState );
	if ( iter == m_verbPropertiesMap.end() )
	{
		return nullptr;
	}

	return iter->second;
}


//-----------------------------------------------------------------------------------------------
ItemDefinition* ItemDefinition::GetItemDefinition( const std::string& itemName )
{
	std::map< std::string, ItemDefinition* >::const_iterator  mapIter = ItemDefinition::s_definitions.find( itemName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}
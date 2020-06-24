#include "Game/ItemDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteAnimSetDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, ItemDefinition* > ItemDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
ItemDefinition::ItemDefinition( const XmlElement& itemDefElem )
	: EntityDefinition( itemDefElem )
{
	const XmlElement* spriteAnimSetElement = itemDefElem.FirstChildElement( "SpriteAnimSet" );
	if ( spriteAnimSetElement != nullptr )
	{
		m_spriteAnimSetDef = new SpriteAnimSetDefinition( *g_renderer, *spriteAnimSetElement );
	}

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

		NamedProperties* properties = new NamedProperties();
	
		eVerbState verbState = GetVerbStateFromString( type );
		properties->SetValue( "EventName", GetEventNameForVerbState( verbState ) );
		properties->SetValue( "Texture", (void*)texture );
		properties->SetValue( "InitialDialogueState", initialDialogueState );
		properties->SetValue( "RequiredItem", requiredItemName );

		m_verbPropertiesMap[verbState] = properties;

		actionEventElem = actionEventElem->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
ItemDefinition::~ItemDefinition()
{
	PTR_SAFE_DELETE( m_spriteAnimSetDef );
	PTR_MAP_SAFE_DELETE( m_verbPropertiesMap );
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* ItemDefinition::GetSpriteAnimDef( const std::string& animName )
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

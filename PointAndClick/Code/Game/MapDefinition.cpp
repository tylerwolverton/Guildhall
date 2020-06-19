#include "Game/MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Actor.hpp"
#include "Game/Item.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ItemDefinition.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, MapDefinition* > MapDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
EntityVector MapDefinition::GetEntitiesInLevel()
{
	EntityVector entitiesInLevel;
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity* const& entity = m_entities[entityIndex];
		if ( !entity->IsInPlayerInventory() )
		{
			entitiesInLevel.push_back( entity );
		}
	}

	return entitiesInLevel;
}


//-----------------------------------------------------------------------------------------------
std::vector<Item*> MapDefinition::GetItemsInLevel()
{
	std::vector<Item*> itemsInLevel;
	for ( int itemIdx = 0; itemIdx < (int)m_items.size(); ++itemIdx )
	{
		Item* const& item = m_items[itemIdx];
		if ( !item->IsInPlayerInventory() )
		{
			itemsInLevel.push_back( item );
		}
	}

	return itemsInLevel;
}


//-----------------------------------------------------------------------------------------------
MapDefinition* MapDefinition::GetMapDefinition( std::string mapName )
{
	std::map< std::string, MapDefinition* >::const_iterator  mapIter = MapDefinition::s_definitions.find( mapName );
	
	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
MapDefinition::MapDefinition( const XmlElement& mapDefElem )
{
	m_name = ParseXmlAttribute( mapDefElem, "name", "" );
	GUARANTEE_OR_DIE( m_name != "", "Map did not have a name attribute" );

	m_width = ParseXmlAttribute( mapDefElem, "width", m_width );
	m_height = ParseXmlAttribute( mapDefElem, "height", m_height );

	std::string backgroundTexturePath = ParseXmlAttribute( mapDefElem, "backgroundTexturePath", "" );

	if ( backgroundTexturePath.empty() )
	{
		m_backgroundTexture = g_renderer->GetDefaultWhiteTexture();
	}
	else
	{
		m_backgroundTexture = g_renderer->CreateOrGetTextureFromFile( backgroundTexturePath.c_str() );
	}

	const XmlElement* entitiesElem = mapDefElem.FirstChildElement( "Entities" );
	if ( entitiesElem != nullptr )
	{
		const XmlElement* entityElem = entitiesElem->FirstChildElement( "Entity" );
		while ( entityElem != nullptr )
		{
			std::string name = ParseXmlAttribute( *entityElem, "name", "" );
			Vec2 pos = ParseXmlAttribute( *entityElem, "startPos", Vec2::ZERO );
			std::string type = ParseXmlAttribute( *entityElem, "type", "actor" );

			if ( type == "actor" )
			{
				Actor* newActor = new Actor( pos, ActorDefinition::GetActorDefinition( name ) );

				m_entities.push_back( newActor );
			}
			else if ( type == "item" )
			{
				Item* newItem = new Item( pos, ItemDefinition::GetItemDefinition( name ) );

				m_entities.push_back( newItem );
				m_items.push_back( newItem );
			}

			entityElem = entityElem->NextSiblingElement();
		}
	}
}


//-----------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{
}

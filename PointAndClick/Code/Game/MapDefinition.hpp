#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class Map;
class Item;
class Texture;


//-----------------------------------------------------------------------------------------------
class MapDefinition
{
	friend class Map;

public:
	explicit MapDefinition( const XmlElement& mapDefElem );
	~MapDefinition();

	std::string GetName()													{ return m_name; }
	EntityVector GetEntitiesInLevel();
	std::vector<Item*> GetItemsInLevel();

	static MapDefinition* GetMapDefinition( std::string mapName );

public:
	static std::map< std::string, MapDefinition* > s_definitions;

private:
	std::string		m_name;
	int				m_width = 0;
	int				m_height = 0;

	EntityVector	m_entities;
	std::vector<Item*>	m_items;

	Texture*		m_backgroundTexture = nullptr;
};

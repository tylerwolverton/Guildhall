#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class Map;
class Portal;
class Texture;


//-----------------------------------------------------------------------------------------------
class MapDefinition
{
	friend class Map;

public:
	explicit MapDefinition( const XmlElement& mapDefElem );
	~MapDefinition();

	std::string GetName() const												{ return m_name; }
	Vec2 GetPlayerStartPos() const											{ return m_playerStartPos; }
	EntityVector GetEntitiesInLevel();
	std::vector<Entity*> GetItemsInLevel();
	std::vector<Portal*> GetPortalsInLevel();

	static MapDefinition* GetMapDefinition( std::string mapName );

public:
	static std::map< std::string, MapDefinition* > s_definitions;

private:
	std::string			 m_name;
	int					 m_width = 0;
	int					 m_height = 0;

	Vec2				 m_playerStartPos;

	EntityVector		 m_entities;
	std::vector<Entity*> m_items;
	std::vector<Portal*> m_portals;

	Texture*			 m_backgroundTexture = nullptr;
};

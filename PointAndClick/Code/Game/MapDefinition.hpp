#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class Map;
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

	static MapDefinition* GetMapDefinition( std::string mapName );

public:
	static std::map< std::string, MapDefinition* > s_definitions;

private:
	std::string		m_name;
	int				m_width = 0;
	int				m_height = 0;

	EntityVector	m_entities;

	Texture*		m_backgroundTexture = nullptr;
};

#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class TileDefinition;
class Map;


//-----------------------------------------------------------------------------------------------
class MapDefinition
{
	friend class Map;
	friend class TileMap;

public:
	explicit MapDefinition( const XmlElement& mapDefElem );
	explicit MapDefinition( const XmlElement& mapDefElem, const std::string& name );
	~MapDefinition();

	std::string GetName()													{ return m_name; }

	static MapDefinition* GetMapDefinition( std::string mapName );

public:
	static std::map< std::string, MapDefinition* > s_definitions;

private:
	std::string m_name;
	IntVec2 m_dimensions;
};

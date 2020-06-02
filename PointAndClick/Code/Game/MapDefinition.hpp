#pragma once
#include "Engine/Core/XmlUtils.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class TileDefinition;
class Map;
class MapGenStep;


//-----------------------------------------------------------------------------------------------
class MapDefinition
{
	friend class Map;

public:
	explicit MapDefinition( const XmlElement& mapDefElem );
	~MapDefinition();

	std::string GetName()													{ return m_name; }
	void RunMapGenerationSteps( Map& map );

	static MapDefinition* GetMapDefinition( std::string mapName );

public:
	static std::map< std::string, MapDefinition* > s_definitions;

private:
	std::string m_name;
	int m_width = 0;
	int m_height = 0;
	TileDefinition* m_fillTile = nullptr;
	TileDefinition* m_edgeTile = nullptr;
	std::vector< MapGenStep* > m_mapGenerationSteps;
};
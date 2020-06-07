#include "Game/Map.hpp"

#include "Game/MapDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( std::string name, MapDefinition* mapDef )
	: m_name( name )
	, m_mapDef( mapDef )
{
	m_dimensions = mapDef->m_dimensions;
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
}





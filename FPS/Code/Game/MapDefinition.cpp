#include "Game/MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/MapGenStep.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, MapDefinition* > MapDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
void MapDefinition::RunMapGenerationSteps( Map& map )
{
	for ( int mapGenStepIndex = 0; mapGenStepIndex < (int)m_mapGenerationSteps.size(); ++mapGenStepIndex )
	{
		m_mapGenerationSteps[mapGenStepIndex]->RunStep( map );
	}
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

	std::string fillTileName = ParseXmlAttribute( mapDefElem, "fillTile", "" );
	if ( !fillTileName.empty() )
	{
		m_fillTile = TileDefinition::GetTileDefinition( fillTileName );
	}

	std::string edgeTileName = ParseXmlAttribute( mapDefElem, "edgeTile", "" );
	if ( !edgeTileName.empty() )
	{
		m_edgeTile = TileDefinition::GetTileDefinition( edgeTileName );
	}

	const XmlElement* mapGenStepsElem = mapDefElem.FirstChildElement( "MapGenSteps" );
	if ( mapGenStepsElem != nullptr )
	{
		const XmlElement* mapGenStepElem = mapGenStepsElem->FirstChildElement();
		while ( mapGenStepElem )
		{
			MapGenStep* mapGenStep = MapGenStep::CreateNewMapGenStep( *mapGenStepElem );
			m_mapGenerationSteps.push_back( mapGenStep );

			mapGenStepElem = mapGenStepElem->NextSiblingElement();
		}
	}
}


//-----------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{
	for ( int mapGenStepIndex = 0; mapGenStepIndex < (int)m_mapGenerationSteps.size(); ++mapGenStepIndex )
	{
		delete m_mapGenerationSteps[mapGenStepIndex];
		m_mapGenerationSteps[mapGenStepIndex] = nullptr;
	}

	m_mapGenerationSteps.clear();
}

#include "Game/MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapRegionTypeDefinition.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, MapDefinition* > MapDefinition::s_definitions;


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
	m_dimensions = ParseXmlAttribute( mapDefElem, "dimensions", m_dimensions );

	
}


//-----------------------------------------------------------------------------------------------
MapDefinition::MapDefinition( const XmlElement& mapDefElem, const std::string& name )
	: m_name( name )
{
	m_type = ParseXmlAttribute( mapDefElem, "type", m_type );
	// TODO: Check type exists
	m_version = ParseXmlAttribute( mapDefElem, "version", m_version );
	// TODO: Check version exists
	m_dimensions = ParseXmlAttribute( mapDefElem, "dimensions", m_dimensions );
	// TODO: Check dimensions exists

	const XmlElement* legendElem = mapDefElem.FirstChildElement( "Legend" );
	// TODO: Check legend exists
	const XmlElement* tileElem = legendElem->FirstChildElement( "Tile" );
	// TODO: Check tile exists
	while ( tileElem )
	{
		//char glyph = ParseXmlAttribute( *tileElem, "glyph", "" )[0];
		char glyph = ParseXmlAttribute( *tileElem, "glyph", ' ' );
		std::string regionTypeStr = ParseXmlAttribute( *tileElem, "regionType", "InvalidRegion" );
		// TODO: Check no repeated glyphs, all regions exist

		m_legend[glyph] = regionTypeStr;

		tileElem = tileElem->NextSiblingElement();
	}
	
	m_regionTypeDefs.resize( (size_t)m_dimensions.x * (size_t)m_dimensions.y );

	const XmlElement* mapRowsElem = mapDefElem.FirstChildElement( "MapRows" );
	const XmlElement* mapRowElem = mapRowsElem->FirstChildElement( "MapRow" );
	int rowNum = m_dimensions.y - 1;
	while ( mapRowElem )
	{
		std::string tilesStr = ParseXmlAttribute( *mapRowElem, "tiles", "" );
		// TODO: Check right length
		for ( int regionDefNum = 0; regionDefNum < tilesStr.length(); ++regionDefNum )
		{
			std::string region = m_legend[tilesStr[regionDefNum]];

			MapRegionTypeDefinition* regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( region );

			/*bool isSolid = false;
			if ( region == "CobblestoneWall" )
			{
				isSolid = true;
			}
			else if ( region == "StoneFloor" )
			{
				isSolid = false;
			}*/

			int tileIdx = ( rowNum * m_dimensions.x ) + regionDefNum;
			//m_regionTypeDefs[tileIdx].m_isSolid = isSolid;
			m_regionTypeDefs[tileIdx] = regionDef;
		}

		mapRowElem = mapRowElem->NextSiblingElement();
		--rowNum;
	}

	const XmlElement* entitiesElem = mapDefElem.FirstChildElement( "Entities" );
	const XmlElement* entityElem = entitiesElem->FirstChildElement();
	while ( entityElem )
	{
		if ( !strcmp( entityElem->Value(), "PlayerStart" ) )
		{
			m_playerStartPos = ParseXmlAttribute( *entityElem, "pos", m_playerStartPos );
			m_playerStartYaw = ParseXmlAttribute( *entityElem, "yaw", m_playerStartYaw );
		}

		entityElem = entityElem->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{	
}

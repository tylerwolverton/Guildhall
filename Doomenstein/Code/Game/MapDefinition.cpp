#include "Game/MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
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
MapDefinition::MapDefinition( const XmlElement& mapDefElem, const std::string& name )
	: m_name( name )
{
	if ( !ParseMapDefinitionNode( mapDefElem ) ) { return; }
	ParseLegendNode( mapDefElem );
	//if ( !ParseLegendNode( mapDefElem ) )		 { return; }
	
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
			if ( regionDef == nullptr )
			{
				regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( "InvalidRegion" );
			}
			
			int tileIdx = ( rowNum * m_dimensions.x ) + regionDefNum;
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


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseMapDefinitionNode( const XmlElement& mapDefElem )
{
	m_type = ParseXmlAttribute( mapDefElem, "type", m_type );
	if ( m_type == "InvalidType" )
	{
		g_devConsole->PrintError( Stringf( "Map file %s is missing a type attribute", m_name.c_str() ) );
	}

	m_version = ParseXmlAttribute( mapDefElem, "version", m_version );
	if ( m_version == -1 )
	{
		g_devConsole->PrintError( Stringf( "Map file %s is missing a version attribute", m_name.c_str() ) );
	}

	m_dimensions = ParseXmlAttribute( mapDefElem, "dimensions", m_dimensions );
	if ( m_dimensions == IntVec2::ZERO )
	{
		g_devConsole->PrintError( Stringf( "Map file %s is missing a dimensions attribute", m_name.c_str() ) );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseLegendNode( const XmlElement& mapDefElem )
{
	m_regionTypeDefs.resize( (size_t)m_dimensions.x * (size_t)m_dimensions.y );

	const XmlElement* legendElem = mapDefElem.FirstChildElement( "Legend" );
	if ( legendElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file %s is missing a legend", m_name.c_str() ) );
		return false;
	}

	const XmlElement* tileElem = legendElem->FirstChildElement( "Tile" );
	if ( m_type == "InvalidType" )
	{
		g_devConsole->PrintError( Stringf( "Map file %s is missing a type attribute", m_name.c_str() ) );
	}

	while ( tileElem )
	{
		//char glyph = ParseXmlAttribute( *tileElem, "glyph", "" )[0];
		char glyph = ParseXmlAttribute( *tileElem, "glyph", ' ' );
		std::string regionTypeStr = ParseXmlAttribute( *tileElem, "regionType", "InvalidRegion" );
		// TODO: Check no repeated glyphs, all regions exist

		m_legend[glyph] = regionTypeStr;

		tileElem = tileElem->NextSiblingElement();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseMapRowsNode( const XmlElement& mapDefElem )
{
	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseEntitiesNode( const XmlElement& mapDefElem )
{
	return true;
}

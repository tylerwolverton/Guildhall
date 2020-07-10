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
MapDefinition::MapDefinition( const XmlElement& mapDefElem, const std::string& mapName, const std::string& defaultRegionName )
	: m_name( mapName )
	, m_defaultRegionTypeStr( defaultRegionName )
{
	if ( !ParseMapDefinitionNode( mapDefElem ) ) { return; }

	std::map<char, MapRegionTypeDefinition*> legend;
	if ( !ParseLegendNode( mapDefElem, legend ) ) { return; }
	if ( !ParseMapRowsNode( mapDefElem, legend ) ) { return; }
	
	if ( !ParseEntitiesNode( mapDefElem ) ) { return; }

	m_isValid = true;
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
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a type attribute", m_name.c_str() ) );
		return false;
	}

	m_version = ParseXmlAttribute( mapDefElem, "version", m_version );
	if ( m_version == -1 )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a version attribute", m_name.c_str() ) );
		return false;
	}

	m_dimensions = ParseXmlAttribute( mapDefElem, "dimensions", m_dimensions );
	if ( m_dimensions == IntVec2::ZERO )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a dimensions attribute", m_name.c_str() ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseLegendNode( const XmlElement& mapDefElem, std::map<char, MapRegionTypeDefinition*>& legend )
{
	m_regionTypeDefs.resize( (size_t)m_dimensions.x * (size_t)m_dimensions.y );

	const XmlElement* legendElem = mapDefElem.FirstChildElement( "Legend" );
	if ( legendElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a legend", m_name.c_str() ) );
		return false;
	}

	const XmlElement* tileElem = legendElem->FirstChildElement( "Tile" );
	if ( tileElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no Tile nodes in legend", m_name.c_str() ) );
	}

	while ( tileElem )
	{
		char glyph = ParseXmlAttribute( *tileElem, "glyph", ' ' );
		if ( legend.find( glyph ) != legend.end() )
		{
			g_devConsole->PrintError( Stringf( "Glyph '%c' has already been defined in legend, first definition will be used", glyph ) );
			tileElem = tileElem->NextSiblingElement();
			continue;
		}

		std::string regionTypeStr = ParseXmlAttribute( *tileElem, "regionType", "" );
		MapRegionTypeDefinition* regionDef = nullptr;
		if ( regionTypeStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Glyph '%c' is missing a regionType attribute in legend", glyph ) );
			regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( m_defaultRegionTypeStr );
		}
		else
		{
			regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( regionTypeStr );
			if ( regionDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Glyph '%c' references unknown regionType '%s'", glyph, regionTypeStr.c_str() ) );
				regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( m_defaultRegionTypeStr );
			}
		}

		legend[glyph] = regionDef;

		tileElem = tileElem->NextSiblingElement();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseMapRowsNode( const XmlElement& mapDefElem, const std::map<char, MapRegionTypeDefinition*>& legend )
{
	const XmlElement* mapRowsElem = mapDefElem.FirstChildElement( "MapRows" );
	if ( mapRowsElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no MapRows node", m_name.c_str() ) );
		return false;
	}

	const XmlElement* mapRowElem = mapRowsElem->FirstChildElement( "MapRow" );
	if ( mapRowsElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no MapRow nodes defined", m_name.c_str() ) );
		return false;
	}

	int rowNum = m_dimensions.y - 1;
	int numRowsDefined = 0;
	while ( mapRowElem )
	{
		++numRowsDefined;
		if ( numRowsDefined > m_dimensions.y )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': Too many MapRows defined. Expected: %d", m_name.c_str(), m_dimensions.y ) );
			return false;
		}

		std::string tilesStr = ParseXmlAttribute( *mapRowElem, "tiles", "" );
		if ( tilesStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d is missing a tiles attribute", m_name.c_str(), numRowsDefined ) );
			return false;
		}

		if ( (int)tilesStr.size() != m_dimensions.x )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d has incorrect number of tiles defined. Expected: %d  Actual: %d", m_name.c_str(), numRowsDefined, m_dimensions.x, (int)tilesStr.size() ) );
			return false;
		}

		for ( int regionDefNum = 0; regionDefNum < tilesStr.length(); ++regionDefNum )
		{
			MapRegionTypeDefinition* regionDef = nullptr;
			auto it = legend.find(tilesStr[regionDefNum]);
			if ( it == legend.end() )
			{
				g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d contains unknown glyph '%c'", m_name.c_str(), numRowsDefined, tilesStr[regionDefNum] ) );
				regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( m_defaultRegionTypeStr );
			}
			else
			{
				regionDef = it->second;
			}

			int tileIdx = ( rowNum * m_dimensions.x ) + regionDefNum;
			m_regionTypeDefs[tileIdx] = regionDef;
		}

		mapRowElem = mapRowElem->NextSiblingElement();
		--rowNum;
	}

	if ( numRowsDefined < m_dimensions.y )
	{
		g_devConsole->PrintError( Stringf( "Map '%s': Not enough MapRows defined. Expected: %d  Actual: %d", m_name.c_str(), m_dimensions.y, numRowsDefined ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseEntitiesNode( const XmlElement& mapDefElem )
{
	const XmlElement* entitiesElem = mapDefElem.FirstChildElement( "Entities" );
	if ( entitiesElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing an entities node", m_name.c_str() ) );
		return false;
	}

	bool hasParsedPlayerStart = false;
	const XmlElement* entityElem = entitiesElem->FirstChildElement();
	while ( entityElem )
	{
		if ( !strcmp( entityElem->Value(), "PlayerStart" ) )
		{
			if ( hasParsedPlayerStart )
			{
				g_devConsole->PrintError( Stringf( "Map file '%s' defines multiple player start nodes in Entities, the first one will be used", m_name.c_str() ) );
				continue;
			}

			m_playerStartPos = ParseXmlAttribute( *entityElem, "pos", m_playerStartPos );
			m_playerStartYaw = ParseXmlAttribute( *entityElem, "yaw", m_playerStartYaw );

			hasParsedPlayerStart = true;
		}
		else if ( !strcmp( entityElem->Value(), "Actor" ) )
		{
			MapEntityDefinition mapEntityDef;

			std::string actorName = ParseXmlAttribute( *entityElem, "name", "" );
			mapEntityDef.entityDef = EntityDefinition::GetEntityDefinition( actorName );
			if ( mapEntityDef.entityDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Map file '%s': Entity '%s' was not defined in EntityTypes.xml", m_name.c_str(), actorName.c_str() ) );
				entityElem = entityElem->NextSiblingElement();
				continue;
			}

			if ( mapEntityDef.entityDef->GetType() != eEntityType::ACTOR )
			{
				g_devConsole->PrintError( Stringf( "Entity '%s' was defined as '%s' in EntityTypes.xml, but Actor in map '%s'", actorName.c_str(), 
																																GetEntityTypeAsString( mapEntityDef.entityDef->GetType() ).c_str(), 
																																m_name.c_str() ) );
				entityElem = entityElem->NextSiblingElement();
				continue;
			}

			mapEntityDef.position = ParseXmlAttribute( *entityElem, "pos", Vec2::ZERO );
			mapEntityDef.yawDegrees = ParseXmlAttribute( *entityElem, "yaw", 0.f ); 

			m_mapEntityDefs.push_back( mapEntityDef );
		}
		else if ( !strcmp( entityElem->Value(), "Portal" ) )
		{
			MapEntityDefinition mapEntityDef;

			std::string portalName = ParseXmlAttribute( *entityElem, "name", "" );
			mapEntityDef.entityDef = EntityDefinition::GetEntityDefinition( portalName );
			if ( mapEntityDef.entityDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Map file '%s': Entity '%s' was not defined in EntityTypes.xml", m_name.c_str(), portalName.c_str() ) );
				entityElem = entityElem->NextSiblingElement();
				continue;
			}

			if ( mapEntityDef.entityDef->GetType() != eEntityType::PORTAL )
			{
				g_devConsole->PrintError( Stringf( "Entity '%s' was defined as '%s' in EntityTypes.xml, but Portal in map '%s'", portalName.c_str(), 
																																 GetEntityTypeAsString( mapEntityDef.entityDef->GetType() ).c_str(), 
																																 m_name.c_str() ) );
				entityElem = entityElem->NextSiblingElement();
				continue;
			}

			mapEntityDef.position = ParseXmlAttribute( *entityElem, "pos", Vec2::ZERO );
			mapEntityDef.yawDegrees = ParseXmlAttribute( *entityElem, "yaw", 0.f );
			mapEntityDef.portalDestMap = ParseXmlAttribute( *entityElem, "destMap", "" );
			mapEntityDef.portalDestPos = ParseXmlAttribute( *entityElem, "destPos", Vec2::ZERO );
			mapEntityDef.portalDestYawOffset = ParseXmlAttribute( *entityElem, "destYawOffset", 0.f );


			m_mapEntityDefs.push_back( mapEntityDef );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "Entity type '%s' is unknown", entityElem->Value() ) );
		}

		entityElem = entityElem->NextSiblingElement();
	}

	if ( hasParsedPlayerStart == false )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a player start node in Entities", m_name.c_str() ) );
		return false;
	}

	return true;
}

#include "Game/MapData.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"

#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
MapData::MapData( const XmlElement& mapDefElem, const std::string& mapName, const std::string& defaultTileName )
	: mapName( mapName )
{
	if ( !ParseMapDefinitionNode( mapDefElem ) ) { return; }

	std::map<char, TileDefinition*> legend;
	if ( !ParseLegendNode( mapDefElem, legend, defaultTileName ) ) { return; }
	if ( !ParseMapRowsNode( mapDefElem, legend, defaultTileName ) ) { return; }
	if ( !ParseEntitiesNode( mapDefElem ) ) { return; }

	isValid = true;
}


//-----------------------------------------------------------------------------------------------
MapData::~MapData()
{	
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseMapDefinitionNode( const XmlElement& mapDefElem )
{
	type = ParseXmlAttribute( mapDefElem, "type", type );
	if ( type == "InvalidType" )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a type attribute", mapName.c_str() ) );
		return false;
	}

	version = ParseXmlAttribute( mapDefElem, "version", version );
	if ( version == -1 )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a version attribute", mapName.c_str() ) );
		return false;
	}

	dimensions = ParseXmlAttribute( mapDefElem, "dimensions", dimensions );
	if ( dimensions == IntVec2::ZERO )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a dimensions attribute", mapName.c_str() ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseLegendNode( const XmlElement& mapDefElem, std::map<char, TileDefinition*>& legend, const std::string& defaultTileName )
{
	tileDefs.resize( (size_t)dimensions.x * (size_t)dimensions.y );

	const XmlElement* legendElem = mapDefElem.FirstChildElement( "Legend" );
	if ( legendElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a legend", mapName.c_str() ) );
		return false;
	}

	const XmlElement* tileElem = legendElem->FirstChildElement( "Tile" );
	if ( tileElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no Tile nodes in legend", mapName.c_str() ) );
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

		std::string tileTypeStr = ParseXmlAttribute( *tileElem, "tileType", "" );
		TileDefinition* tileDef = nullptr;
		if ( tileTypeStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Glyph '%c' is missing a tileType attribute in legend", glyph ) );
			tileDef = TileDefinition::GetTileDefinition( defaultTileName );
		}
		else
		{
			tileDef = TileDefinition::GetTileDefinition( tileTypeStr );
			if ( tileDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Glyph '%c' references unknown tileType '%s'", glyph, tileTypeStr.c_str() ) );
				tileDef = TileDefinition::GetTileDefinition( defaultTileName );
			}
		}

		legend[glyph] = tileDef;

		tileElem = tileElem->NextSiblingElement();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseMapRowsNode( const XmlElement& mapDefElem, const std::map<char, TileDefinition*>& legend, const std::string& defaultTileName )
{
	const XmlElement* mapRowsElem = mapDefElem.FirstChildElement( "MapRows" );
	if ( mapRowsElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no MapRows node", mapName.c_str() ) );
		return false;
	}

	const XmlElement* mapRowElem = mapRowsElem->FirstChildElement( "MapRow" );
	if ( mapRowsElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no MapRow nodes defined", mapName.c_str() ) );
		return false;
	}

	int rowNum = dimensions.y - 1;
	int numRowsDefined = 0;
	while ( mapRowElem )
	{
		++numRowsDefined;
		if ( numRowsDefined > dimensions.y )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': Too many MapRows defined. Expected: %d", mapName.c_str(), dimensions.y ) );
			return false;
		}

		std::string tilesStr = ParseXmlAttribute( *mapRowElem, "tiles", "" );
		if ( tilesStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d is missing a tiles attribute", mapName.c_str(), numRowsDefined ) );
			return false;
		}

		if ( (int)tilesStr.size() != dimensions.x )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d has incorrect number of tiles defined. Expected: %d  Actual: %d", mapName.c_str(), numRowsDefined, dimensions.x, (int)tilesStr.size() ) );
			return false;
		}

		for ( int tileDefNum = 0; tileDefNum < tilesStr.length(); ++tileDefNum )
		{
			TileDefinition* tileDef = nullptr;
			auto it = legend.find( tilesStr[tileDefNum] );
			if ( it == legend.end() )
			{
				g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d contains unknown glyph '%c'", mapName.c_str(), numRowsDefined, tilesStr[tileDefNum] ) );
				tileDef = TileDefinition::GetTileDefinition( defaultTileName );
			}
			else
			{
				tileDef = it->second;
			}

			int tileIdx = ( rowNum * dimensions.x ) + tileDefNum;
			tileDefs[tileIdx] = tileDef;
		}

		mapRowElem = mapRowElem->NextSiblingElement();
		--rowNum;
	}

	if ( numRowsDefined < dimensions.y )
	{
		g_devConsole->PrintError( Stringf( "Map '%s': Not enough MapRows defined. Expected: %d  Actual: %d", mapName.c_str(), dimensions.y, numRowsDefined ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseEntitiesNode( const XmlElement& mapDefElem )
{
	const XmlElement* entitiesElem = mapDefElem.FirstChildElement( "Entities" );
	if ( entitiesElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing an entities node", mapName.c_str() ) );
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
				g_devConsole->PrintError( Stringf( "Map file '%s' defines multiple player start nodes in Entities, the first one will be used", mapName.c_str() ) );
				continue;
			}

			playerStartPos = ParseXmlAttribute( *entityElem, "pos", playerStartPos );
			playerStartYaw = ParseXmlAttribute( *entityElem, "yaw", playerStartYaw );

			hasParsedPlayerStart = true;
		}
		else if ( !strcmp( entityElem->Value(), "Actor" ) )
		{
			MapEntityDefinition mapEntityDef;

			std::string actorName = ParseXmlAttribute( *entityElem, "name", "" );
			mapEntityDef.entityDef = EntityDefinition::GetEntityDefinition( actorName );
			if ( mapEntityDef.entityDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Map file '%s': Entity '%s' was not defined in EntityTypes.xml", mapName.c_str(), actorName.c_str() ) );
				entityElem = entityElem->NextSiblingElement();
				continue;
			}

			if ( mapEntityDef.entityDef->GetType() != eEntityType::ACTOR )
			{
				g_devConsole->PrintError( Stringf( "Entity '%s' was defined as '%s' in EntityTypes.xml, but Actor in map '%s'", actorName.c_str(),
												   GetEntityTypeAsString( mapEntityDef.entityDef->GetType() ).c_str(),
												   mapName.c_str() ) );
				entityElem = entityElem->NextSiblingElement();
				continue;
			}

			mapEntityDef.position = ParseXmlAttribute( *entityElem, "pos", Vec2::ZERO );
			mapEntityDef.yawDegrees = ParseXmlAttribute( *entityElem, "yaw", 0.f );

			mapEntityDefs.push_back( mapEntityDef );
		}
		else if ( !strcmp( entityElem->Value(), "Portal" ) )
		{
			MapEntityDefinition mapEntityDef;

			std::string portalName = ParseXmlAttribute( *entityElem, "name", "" );
			mapEntityDef.entityDef = EntityDefinition::GetEntityDefinition( portalName );
			if ( mapEntityDef.entityDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Map file '%s': Entity '%s' was not defined in EntityTypes.xml", mapName.c_str(), portalName.c_str() ) );
				entityElem = entityElem->NextSiblingElement();
				continue;
			}

			if ( mapEntityDef.entityDef->GetType() != eEntityType::PORTAL )
			{
				g_devConsole->PrintError( Stringf( "Entity '%s' was defined as '%s' in EntityTypes.xml, but Portal in map '%s'", portalName.c_str(),
												   GetEntityTypeAsString( mapEntityDef.entityDef->GetType() ).c_str(),
												   mapName.c_str() ) );
				entityElem = entityElem->NextSiblingElement();
				continue;
			}

			mapEntityDef.position = ParseXmlAttribute( *entityElem, "pos", Vec2::ZERO );
			mapEntityDef.yawDegrees = ParseXmlAttribute( *entityElem, "yaw", 0.f );
			mapEntityDef.portalDestMap = ParseXmlAttribute( *entityElem, "destMap", "" );
			mapEntityDef.portalDestPos = ParseXmlAttribute( *entityElem, "destPos", Vec2::ZERO );
			mapEntityDef.portalDestYawOffset = ParseXmlAttribute( *entityElem, "destYawOffset", 0.f );


			mapEntityDefs.push_back( mapEntityDef );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "Entity type '%s' is unknown", entityElem->Value() ) );
		}

		entityElem = entityElem->NextSiblingElement();
	}

	if ( hasParsedPlayerStart == false )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a player start node in Entities", mapName.c_str() ) );
		return false;
	}

	return true;
}
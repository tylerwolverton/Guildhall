#include "Game/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/TileMap.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"


//-----------------------------------------------------------------------------------------------
World::World( Clock* gameClock )
{
	m_worldClock = new Clock( gameClock );
}


//-----------------------------------------------------------------------------------------------
World::~World()
{
	PTR_SAFE_DELETE( m_worldClock );

	std::map< std::string, Map* >::iterator it;

	for ( it = m_loadedMaps.begin(); it != m_loadedMaps.end(); it++ )
	{
		PTR_SAFE_DELETE( it->second );
	}

	m_loadedMaps.clear();
}


//-----------------------------------------------------------------------------------------------
void World::Update()
{
	if ( m_curMap == nullptr )
	{
		return;
	}

	m_curMap->UpdateMeshes();
}


//-----------------------------------------------------------------------------------------------
void World::Render() const
{
	if ( m_curMap == nullptr )
	{
		return;
	}

	m_curMap->Render();
}


//-----------------------------------------------------------------------------------------------
void World::DebugRender() const
{
	if ( m_curMap == nullptr )
	{
		return;
	}

	m_curMap->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void World::LoadMap( const std::string& mapName )
{
	Map* mapIter = GetLoadedMapByName( mapName );
	if ( mapIter != nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map '%s' has already been loaded", mapName.c_str() ) );
		return;
	}

	// Only load maps that have already been parsed and added to MapDefinitions
	MapDefinition* mapDef = MapDefinition::GetMapDefinition( mapName );
	if ( mapDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map '%s' has not been loaded from Maps directory", mapName.c_str() ) );
		return;
	}

	if ( mapDef->GetType() == "TileMap" )
	{
		TileMap* tileMap = new TileMap( mapName, mapDef );
		m_loadedMaps[mapName] = tileMap;
	}
}


//-----------------------------------------------------------------------------------------------
void World::LoadMap( const std::string& mapName, MapDefinition* mapDef )
{
	if ( mapDef->GetType() == "TileMap" )
	{
		TileMap* tileMap = new TileMap( mapName, mapDef );
		m_loadedMaps[mapName] = tileMap;
	}
}


//-----------------------------------------------------------------------------------------------
void World::ChangeMap( const std::string& mapName )
{
	Map* newMap = GetLoadedMapByName( mapName );
	if ( newMap == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map '%s' has not been loaded into world", mapName.c_str() ) );
		return;
	}

	if ( m_curMap != nullptr )
	{
		m_curMap->Unload();
	}

	m_curMap = newMap;
	
	if ( m_curMap != nullptr )
	{
		m_curMap->Load();
	}
}


//-----------------------------------------------------------------------------------------------
Map* World::GetLoadedMapByName( const std::string& mapName )
{
	auto mapIter = m_loadedMaps.find( mapName );
	if ( mapIter == m_loadedMaps.end() )
	{
		return nullptr;
	}

	return mapIter->second;
}

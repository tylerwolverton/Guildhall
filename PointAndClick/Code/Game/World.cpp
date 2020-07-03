#include "Game/World.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Time/Clock.hpp"
#include "Game/Map.hpp"
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
	PTR_SAFE_DELETE( m_curMap );
}


//-----------------------------------------------------------------------------------------------
void World::Update()
{
	m_curMap->Update( (float)m_worldClock->GetLastDeltaSeconds() );
}


//-----------------------------------------------------------------------------------------------
void World::Render() const
{
	m_curMap->Render();
}


//-----------------------------------------------------------------------------------------------
void World::DebugRender() const
{
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

	Map* map = new Map( mapName, mapDef );
	m_loadedMaps[mapName] = map;
	
}


//-----------------------------------------------------------------------------------------------
void World::ChangeMap( const std::string& mapName, Actor* player )
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
		m_curMap->Load( (Entity*)player );
		g_devConsole->PrintString( Stringf( "Map '%s' loaded", mapName.c_str() ), Rgba8::GREEN );
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


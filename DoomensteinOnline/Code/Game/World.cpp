#include "Game/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/TileMap.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapData.hpp"


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

	m_curMap->Update( (float)m_worldClock->GetLastDeltaSeconds() );
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
void World::AddNewMap( const MapData& mapData )
{
	if ( mapData.type == "TileMap" )
	{
		TileMap* tileMap = new TileMap( mapData );
		m_loadedMaps[mapData.mapName] = tileMap;
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
		g_devConsole->PrintString( Stringf( "Map '%s' loaded", mapName.c_str() ), Rgba8::GREEN );
	}
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist )
{
	if ( m_curMap != nullptr )
	{
		return m_curMap->GetClosestEntityInSector( observerPos, forwardDegrees, apertureDegrees, maxDist );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void World::WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	Map* destMap = GetLoadedMapByName( destMapName );

	// TODO: Verify portal target maps exist while loading xml files
	// Warp to a new map if one is specified and the entity is the player
	if ( destMap != nullptr 
		 && destMap != m_curMap
		 && entityToWarp->IsPossessed() )
	{
		m_curMap->RemoveOwnershipOfEntity( entityToWarp );
		ChangeMap( destMapName );
		m_curMap->TakeOwnershipOfEntity( entityToWarp );
	}

	entityToWarp->SetPosition( newPos );
	entityToWarp->SetOrientationDegrees( newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
bool World::IsMapLoaded( const std::string& mapName )
{
	return GetLoadedMapByName( mapName ) != nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* World::CreateEntityInCurrentMap( const std::string& entityType, const Vec2& position, float yawOrientationDegrees )
{
	if ( m_curMap == nullptr )
	{
		g_devConsole->PrintWarning( Stringf( "Tried to spawn entity of type '%s' in nonexistent current map", entityType.c_str() ) );
		return nullptr;
	}

	Entity* newEntiy = m_curMap->SpawnNewEntityOfType( entityType );
	if ( newEntiy == nullptr )
	{
		return nullptr;
	}

	newEntiy->SetPosition( position );
	newEntiy->SetOrientationDegrees( yawOrientationDegrees );

	return newEntiy;
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

#include "Game/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapData.hpp"
#include "Game/TileMap.hpp"


//-----------------------------------------------------------------------------------------------
World::World( Clock* gameClock )
{
	m_worldClock = new Clock( gameClock );
}


//-----------------------------------------------------------------------------------------------
World::~World()
{
	PTR_SAFE_DELETE( m_worldClock );
	PTR_MAP_SAFE_DELETE( m_loadedMaps );
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
		TileMap* tileMap = new TileMap( mapData, this );
		m_loadedMaps[mapData.mapName] = tileMap;
	}
}


//-----------------------------------------------------------------------------------------------
void World::ChangeMap( const std::string& mapName, Entity* player )
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
		m_curMap->Load( player );
		g_devConsole->PrintString( Stringf( "Map '%s' loaded", mapName.c_str() ), Rgba8::GREEN );
	}
}


//-----------------------------------------------------------------------------------------------
void World::WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	Map* destMap = GetLoadedMapByName( destMapName );

	// TODO: Verify portal target maps exist while loading xml files
	// Warp to a new map if one is specified and the entity is the player
	if ( destMap != nullptr
		 && destMap != m_curMap
		 && entityToWarp->IsPlayer() )
	{
		m_curMap->RemoveOwnershipOfEntity( entityToWarp );
		ChangeMap( destMapName, entityToWarp );
		// ChangeMap will transplant the player
		//m_curMap->TakeOwnershipOfEntity( entityToWarp );
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
void World::UnloadAllEntityScripts()
{
	for ( auto& map : m_loadedMaps )
	{
		map.second->UnloadAllEntityScripts();
	}
}


//-----------------------------------------------------------------------------------------------
void World::ReloadAllEntityScripts()
{
	for ( auto& map : m_loadedMaps )
	{
		map.second->ReloadAllEntityScripts();
	}
}


//-----------------------------------------------------------------------------------------------
void World::ClearMaps()
{
	PTR_MAP_SAFE_DELETE( m_loadedMaps );

	m_curMap = nullptr;
}


//-----------------------------------------------------------------------------------------------
void World::ClearEntities()
{
	m_entitiesByName.clear();
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetEntityById( EntityId id )
{
	// Look in this map first
	Entity* entity = GetEntityByIdInCurMap( id );
	if ( entity != nullptr )
	{
		return entity;
	}

	for ( auto& map : m_loadedMaps )
	{
		if ( map.second == m_curMap )
		{
			continue;
		}

		entity = map.second->GetEntityById( id );
		if ( entity != nullptr )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetEntityByIdInCurMap( EntityId id )
{
	return m_curMap->GetEntityById( id );
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetEntityByName( const std::string& name )
{
	auto entityIter = m_entitiesByName.find( name );
	if ( entityIter != m_entitiesByName.end() )
	{
		return entityIter->second;
	}

	return nullptr;

	// Look in this map first
	/*Entity* entity = GetEntityByNameInCurMap( name );
	if ( entity != nullptr )
	{
		return entity;
	}

	for ( auto& map : m_loadedMaps )
	{
		if ( map.second == m_curMap )
		{
			continue;
		}

		entity = map.second->GetEntityByName( name );
		if ( entity != nullptr )
		{
			return entity;
		}
	}

	return nullptr;*/
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetEntityByNameInCurMap( const std::string& name )
{
	return m_curMap->GetEntityByName( name );
}


//-----------------------------------------------------------------------------------------------
void World::SaveEntityByName( Entity* entity )
{
	if ( entity == nullptr 
		 || entity->GetName().empty() )
	{
		return;
	}

	auto entityIter = m_entitiesByName.find( entity->GetName() );
	if ( entityIter != m_entitiesByName.end() )
	{
		g_devConsole->PrintError( Stringf( "Tried to save an entity with name '%s' in map '%s', but an entity with that name was already defined in map '%s'", 
										   entity->GetName().c_str(), 
										   entity->GetMap()->GetName().c_str(),
										   entityIter->second->GetMap()->GetName().c_str() ) );
		return;
	}

	m_entitiesByName[entity->GetName()] = entity;
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

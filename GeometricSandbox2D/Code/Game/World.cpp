#include "Game/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/Map.hpp"
#include "Game/ConvexRaycastMap.hpp"
#include "Game/GameCommon.hpp"


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
void World::AddNewMap( const std::string& mapName )
{
	Map* newMap;
	if ( mapName == "ConvexRaycast" )
	{
		newMap = new ConvexRaycastMap( mapName );
	}
	else
	{
		newMap = new Map( mapName );
	}
	m_loadedMaps[mapName] = newMap;

	if ( m_loadedMaps.size() == 1 )
	{
		m_curMap = newMap;
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
		g_devConsole->PrintString( Stringf( "Map '%s' loaded", mapName.c_str() ), Rgba8::GREEN );
	}
}


//-----------------------------------------------------------------------------------------------
bool World::IsMapLoaded( const std::string& mapName )
{
	return GetLoadedMapByName( mapName ) != nullptr;
}


//-----------------------------------------------------------------------------------------------
void World::ResetCurrentMap()
{
	m_curMap->Reset();
}


//-----------------------------------------------------------------------------------------------
void World::AddEntitySet( int numEntities )
{
	m_curMap->AddEntitySet( numEntities );
}


//-----------------------------------------------------------------------------------------------
void World::RemoveEntitySet( int numEntities )
{
	m_curMap->RemoveEntitySet( numEntities );
}


//-----------------------------------------------------------------------------------------------
int World::GetObjectCount() const
{
	return m_curMap->GetObjectCount();
}


//-----------------------------------------------------------------------------------------------
int World::GetRaycastCount() const
{
	return m_curMap->GetRaycastCount();
}


//-----------------------------------------------------------------------------------------------
double World::GetRaycastTimeMs() const
{
	return m_curMap->GetRaycastTimeMs();
}


//-----------------------------------------------------------------------------------------------
int World::GetNumRaycastImpacts() const
{
	return m_curMap->GetNumRaycastImpacts();
}


//-----------------------------------------------------------------------------------------------
void World::CycleBroadphaseCheck()
{
	m_curMap->CycleBroadphaseCheck();
}


//-----------------------------------------------------------------------------------------------
eBroadphaseCheckType World::GetBroadphaseCheckType() const
{
	return m_curMap->GetBroadphaseCheckType();
}


//-----------------------------------------------------------------------------------------------
std::string World::GetBroadphaseCheckTypeStr() const
{
	return m_curMap->GetBroadphaseCheckTypeStr();
}


//-----------------------------------------------------------------------------------------------
void World::CycleNarrowphaseCheck()
{
	m_curMap->CycleNarrowphaseCheck();
}


//-----------------------------------------------------------------------------------------------
eNarrowphaseCheckType World::GetNarrowphaseCheckType() const
{
	return m_curMap->GetNarrowphaseCheckType();
}


//-----------------------------------------------------------------------------------------------
std::string World::GetNarrowphaseCheckTypeStr() const
{
	return m_curMap->GetNarowphaseCheckTypeStr();
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


//-----------------------------------------------------------------------------------------------
void World::SaveConvexSceneToFile( const std::string& fileName )
{
	if ( m_curMap->GetName() != "ConvexRaycast" )
	{
		g_devConsole->PrintError( Stringf( "Cannot save convex scene for map type: '%s'", m_curMap->GetName().c_str() ) );
		return;
	}

	( (ConvexRaycastMap*)m_curMap )->SaveConvexSceneToFile( fileName );
}


//-----------------------------------------------------------------------------------------------
void World::LoadConvexSceneFromFile( const std::string& fileName )
{
	if ( m_curMap->GetName() != "ConvexRaycast" )
	{
		g_devConsole->PrintError( Stringf( "Cannot load convex scene data into map of type: '%s'", m_curMap->GetName().c_str() ) );
		return;
	}

	( (ConvexRaycastMap*)m_curMap )->LoadConvexSceneFromFile( fileName );
}

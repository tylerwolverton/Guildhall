#include "Game/World.hpp"
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
void World::BuildNewMap( std::string name )
{
	MapDefinition* mapDef = MapDefinition::GetMapDefinition( name );
	if ( mapDef == nullptr )
	{
		ERROR_AND_DIE( Stringf( "Requested map '%s' is not defined!", name.c_str() ) );
	}
	
	m_curMap = new Map( name, mapDef );
}

#include "Game/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapDefinition.hpp"


//-----------------------------------------------------------------------------------------------
World::World()
{
}


//-----------------------------------------------------------------------------------------------
World::~World()
{
	delete m_curMap;
	m_curMap = nullptr;
}


//-----------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds )
{
	m_curMap->Update( deltaSeconds );
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

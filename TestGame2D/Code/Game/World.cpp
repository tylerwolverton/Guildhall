#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"


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
void World::BuildNewMap( int width, int height )
{
	m_curMap = new Map( width, height );
}

#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"

//-----------------------------------------------------------------------------------------------
World::World( Game* game )
	: m_game(game)
{
	m_curMap = new Map( WORLD_SIZE_X / TILE_SIZE, WORLD_SIZE_Y / TILE_SIZE );
}

//-----------------------------------------------------------------------------------------------
World::~World()
{
	delete m_curMap;
	m_curMap = nullptr;
}

//-----------------------------------------------------------------------------------------------
void World::Render() const
{
	m_curMap->Render();
}

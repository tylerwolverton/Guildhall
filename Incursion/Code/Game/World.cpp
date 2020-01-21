#include "Game/World.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Player.hpp"


//-----------------------------------------------------------------------------------------------
World::World()
{
	TileDefinition::PopulateDefinitions();
}


//-----------------------------------------------------------------------------------------------
World::~World()
{
	// TODO: Just hold a player reference in world?
	if ( m_curMap != nullptr )
	{
		Entity* player = m_curMap->GetPlayer();
		delete player;
		player = nullptr;
	}

	for ( int mapIndex = 0; mapIndex < (int)m_maps.size(); ++mapIndex )
	{
		delete m_maps[mapIndex];
		m_maps[mapIndex] = nullptr;
	}

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
void World::RenderHUD() const
{
	m_curMap->RenderHUD();
}


//-----------------------------------------------------------------------------------------------
void World::DebugRender() const
{
	m_curMap->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void World::GenerateMaps()
{
	for ( int mapIndex = 0; mapIndex < (int)m_maps.size(); ++mapIndex )
	{
		delete m_maps[mapIndex];
		m_maps[mapIndex] = nullptr;
	}
	m_maps.clear();
	m_curMap = nullptr;

	// Map 1
	std::vector<TileWorm> tileWorms1
	{
		TileWorm( TILE_TYPE_MUD, MAP1_MUD_WORM_COUNT, MAP1_MUD_WORM_LENGTH ),
		TileWorm( TILE_TYPE_STONE, MAP1_STONE1_WORM_COUNT, MAP1_STONE1_WORM_LENGTH ),
		TileWorm( TILE_TYPE_STONE, MAP1_STONE2_WORM_COUNT, MAP1_STONE2_WORM_LENGTH )
	};

	MapEntityDefinition map1EntityDef;
	map1EntityDef.m_numNPCTanks = g_game->m_rng->RollRandomIntInRange( MAP1_MIN_TANK_COUNT, MAP1_MAX_TANK_COUNT );
	map1EntityDef.m_numNPCTurrets = g_game->m_rng->RollRandomIntInRange( MAP1_MIN_TURRET_COUNT, MAP1_MAX_TURRET_COUNT );
	map1EntityDef.m_numBoulders = g_game->m_rng->RollRandomIntInRange( MAP1_MIN_BOULDER_COUNT, MAP1_MAX_BOULDER_COUNT );

	MapDefinition mapDef1( IntVec2( MAP1_WIDTH, MAP1_HEIGHT ), TILE_TYPE_GRASS, TILE_TYPE_STONE, TILE_TYPE_GRASS, TILE_TYPE_GRASS, TILE_TYPE_STONE, tileWorms1, map1EntityDef );
	BuildNewMap( mapDef1 );

	// Map 2
	std::vector<TileWorm> tileWorms2
	{
		TileWorm( TILE_TYPE_WET_SAND, MAP2_WET_SAND_WORM_COUNT, MAP2_WET_SAND_WORM_LENGTH ),
		TileWorm( TILE_TYPE_CONCRETE, MAP2_CONCRETE_WORM_COUNT, MAP2_CONCRETE_WORM_LENGTH ),
		TileWorm( TILE_TYPE_WATER, MAP2_WATER1_WORM_COUNT, MAP2_WATER1_WORM_LENGTH ),
		TileWorm( TILE_TYPE_WATER, MAP2_WATER2_WORM_COUNT, MAP2_WATER2_WORM_LENGTH )
	};

	MapEntityDefinition map2EntityDef;
	map2EntityDef.m_numNPCTanks = g_game->m_rng->RollRandomIntInRange( MAP2_MIN_TANK_COUNT, MAP2_MAX_TANK_COUNT );
	map2EntityDef.m_numNPCTurrets = g_game->m_rng->RollRandomIntInRange( MAP2_MIN_TURRET_COUNT, MAP2_MAX_TURRET_COUNT );
	map2EntityDef.m_numBoulders = g_game->m_rng->RollRandomIntInRange( MAP2_MIN_BOULDER_COUNT, MAP2_MAX_BOULDER_COUNT );

	MapDefinition mapDef2( IntVec2( MAP2_WIDTH, MAP2_HEIGHT ), TILE_TYPE_SAND, TILE_TYPE_WATER, TILE_TYPE_SAND, TILE_TYPE_SAND, TILE_TYPE_CONCRETE, tileWorms2, map2EntityDef );
	BuildNewMap( mapDef2 );

	// Map 3
	std::vector<TileWorm> tileWorms3
	{
		TileWorm( TILE_TYPE_WOOD, MAP3_WOOD1_WORM_COUNT, MAP3_WOOD1_WORM_LENGTH ),
		TileWorm( TILE_TYPE_WOOD, MAP3_WOOD2_WORM_COUNT, MAP3_WOOD2_WORM_LENGTH )
	};

	MapEntityDefinition map3EntityDef;
	map3EntityDef.m_numNPCTanks = g_game->m_rng->RollRandomIntInRange( MAP3_MIN_TANK_COUNT, MAP3_MAX_TANK_COUNT );
	map3EntityDef.m_numNPCTurrets = g_game->m_rng->RollRandomIntInRange( MAP3_MIN_TURRET_COUNT, MAP3_MAX_TURRET_COUNT );
	map3EntityDef.m_numBoulders = g_game->m_rng->RollRandomIntInRange( MAP3_MIN_BOULDER_COUNT, MAP3_MAX_BOULDER_COUNT );

	MapDefinition mapDef3( IntVec2( MAP3_WIDTH, MAP3_HEIGHT ), TILE_TYPE_PURPLE_BRICK, TILE_TYPE_WOOD, TILE_TYPE_PURPLE_BRICK, TILE_TYPE_PURPLE_BRICK, TILE_TYPE_WOOD, tileWorms3, map3EntityDef );
	BuildNewMap( mapDef3 );
}


//-----------------------------------------------------------------------------------------------
void World::BuildNewMap( const MapDefinition& mapDefinition )
{
	int nextId = (int)m_maps.size();
	m_maps.push_back( new Map( nextId, this, mapDefinition ) );

	// Set first map as current when added
	if ( (int)m_maps.size() == 1 )
	{
		m_curMap = m_maps[0];
		m_curMap->SpawnNewEntity( ENTITY_TYPE_PLAYER, ENTITY_FACTION_GOOD, Vec2( 2.f, 2.f ), 0.f );
		m_curMap->LoadMap();
	}
}


//-----------------------------------------------------------------------------------------------
void World::ChangeMap( int mapId )
{
	if ( mapId >= (int)m_maps.size() )
	{
		g_game->ChangeGameState( GAME_STATE_VICTORY );
	}
	else
	{
		Entity* player = m_curMap->GetPlayer();
		m_curMap->RemoveEntityFromVector( (Entity*)player, ENTITY_TYPE_PLAYER );
		m_curMap->UnloadMap();

		m_curMap = m_maps[mapId];
		((Player*)player)->SetMap( m_curMap );
		m_curMap->AddEntityToVector( (Entity*)player, ENTITY_TYPE_PLAYER );
		m_curMap->LoadMap();
	}
}


//-----------------------------------------------------------------------------------------------
void World::ChangeMap()
{
	ChangeMap( m_curMap->GetId() + 1 );
}


//-----------------------------------------------------------------------------------------------
Player* World::GetPlayer() const
{
	if ( m_curMap == nullptr )
	{
		return nullptr;
	}

	return (Player*)m_curMap->GetPlayer();
}

#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Game/Tile.hpp"

#include <vector>


//---------------------------------------------------------------------------------------------- -
// Map Generation Helper Structs
struct TileWorm
{
public:
	TileType m_wormTileType = TILE_TYPE_INVALID;
	int m_numWorms = 0;
	int m_wormLength = 0;

public:
	explicit TileWorm( TileType tileType, int numWorms, int wormLength );
};


//----------------------------------------------------------------------------------------------
struct MapEntityDefinition
{
public:
	int m_numNPCTanks = 0;
	int m_numNPCTurrets = 0;
	int m_numBoulders = 0;

public:
	MapEntityDefinition() {} // Do nothing default constructor
	explicit MapEntityDefinition( int numNPCTanks, int numNPCTurrets, int numBoulders );
};


//-----------------------------------------------------------------------------------------------
struct MapDefinition
{
public:
	IntVec2 m_dimensions;
	TileType m_defaultTileType = TILE_TYPE_INVALID;
	TileType m_edgeTileType = TILE_TYPE_INVALID;
	TileType m_startTileType = TILE_TYPE_INVALID;
	TileType m_exitTileType = TILE_TYPE_INVALID;
	TileType m_shieldTileType = TILE_TYPE_INVALID;
	std::vector<TileWorm> m_tileWorms;

	MapEntityDefinition m_mapEntityDefinition;

public:
	explicit MapDefinition( IntVec2 dimensions, TileType defaultTileType, TileType edgeTileType, 
							TileType startTileType, TileType exitTileType, TileType shieldTileType,
							std::vector<TileWorm> tileWorms, 
							const MapEntityDefinition& mapEntityDefinition );
};

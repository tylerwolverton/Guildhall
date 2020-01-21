#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"


//-----------------------------------------------------------------------------------------------
struct AABB2;


//-----------------------------------------------------------------------------------------------
enum TileType
{
	TILE_TYPE_INVALID = -1,

	TILE_TYPE_GRASS,
	TILE_TYPE_STONE,
	TILE_TYPE_MUD,
	TILE_TYPE_SAND, 
	TILE_TYPE_WET_SAND, 
	TILE_TYPE_CONCRETE, 
	TILE_TYPE_WATER,
	TILE_TYPE_LAVA,
	TILE_TYPE_PURPLE_BRICK,
	TILE_TYPE_WOOD,
	TILE_TYPE_MARBLE,
	TILE_TYPE_EXIT,

	NUM_TILE_TYPES
};

Rgba8 GetColorForTileType( TileType tileType );
bool IsTileTypeSolid( TileType tileType );
bool DoesTileTypeStopBullets( TileType tileType );
AABB2 GetUVsForTileType( TileType tileType );


//-----------------------------------------------------------------------------------------------
class Tile
{
public:
	Tile( IntVec2 tileCoords, TileType tileType );
	Tile( int x, int y, TileType tileType );
	~Tile();

	AABB2 GetBounds() const;
	
public:
	IntVec2 m_tileCoords;
	TileType m_tileType;
};

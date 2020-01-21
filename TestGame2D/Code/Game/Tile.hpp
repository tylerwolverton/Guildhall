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

	NUM_TILE_TYPES
};

Rgba8 GetColorForTileType( TileType tileType );
bool IsTileTypeSolid( TileType tileType );


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

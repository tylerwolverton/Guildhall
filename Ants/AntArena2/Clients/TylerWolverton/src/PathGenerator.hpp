#pragma once
#include "Common.hpp"


//-----------------------------------------------------------------------------------------------
struct PathNode
{
public:
	PathNode() = default;
	PathNode( const TileCoords& tileCoords, int gPenalty, int gLast, int h, int idx, int fromNodeIdx )
		: tileCoords( tileCoords )
		, gHere( gPenalty + 1 )
		, h( h )
		, idx( idx )
		, fromNodeIdx( fromNodeIdx )
	{
		gTotal = gHere + gLast;
	}

	TileCoords tileCoords = TILE_COORDS_INVALID;
	int idx = -1;
	int fromNodeIdx = -1;

	int gHere = 1;
	int gTotal = 9999999;
	int h = 999999;

	// TODO: Turn these into flags?
	bool hasBeenProcessed = false;
	//bool isOpen = true;

public:
	int GetFCost() const { return gTotal + h; }
};


//-----------------------------------------------------------------------------------------------
class PathGenerator
{
public:
	static Path		GeneratePath( const TileCoords& startTile, const TileCoords& endTile, const int* tileTypeCosts, const eTileType* observedTileTypes, int mapWidth );
	static int		GetBestOpenNode( std::vector<PathNode>& openTiles );
	static bool		OpenAdjacentTile( std::vector<PathNode>& openTiles, eCardinalDirections direction,
								  const PathNode& curTile, const TileCoords& endTileCoords,
								  const int* tileTypeCosts, const eTileType* observedTileTypes, int mapWidth );
};

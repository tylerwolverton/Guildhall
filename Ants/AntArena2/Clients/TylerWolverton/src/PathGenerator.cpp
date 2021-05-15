#include "PathGenerator.hpp"


//-----------------------------------------------------------------------------------------------
Path PathGenerator::GeneratePath( const TileCoords& startTile, const TileCoords& endTile, const int* tileTypeCosts, const eTileType* observedTileTypes, int mapWidth )
{
	Path pathToGoal;

	TileCoords tileCoordsDiff = endTile - startTile;

	// Heuristic for how far away the goal may be
	int totalH = tileCoordsDiff.GetTaxicabLength();

	// TODO: Turn this into a map by total cost
	std::vector<PathNode> openTiles;
	openTiles.reserve( totalH * 4 );
	openTiles.emplace_back( startTile, 0, 0, totalH, 0, -1 );

	bool foundGoal = false;
	PathNode endNode = openTiles[0];
	while ( !foundGoal 
			|| openTiles.size() > mapWidth * mapWidth * .5f )
	{
		int bestNodeIdx = GetBestOpenNode( openTiles );
		if ( bestNodeIdx == -1
			 || bestNodeIdx > openTiles.size() - 1 )
		{
			return pathToGoal;
		}

		PathNode& nodeToProcess = openTiles[bestNodeIdx];

		if ( OpenAdjacentTile( openTiles, eCardinalDirections::EAST,  nodeToProcess, endTile, tileTypeCosts, observedTileTypes, mapWidth ) ) { endNode = openTiles[openTiles.size() - 1]; break; }
		if ( OpenAdjacentTile( openTiles, eCardinalDirections::WEST,  nodeToProcess, endTile, tileTypeCosts, observedTileTypes, mapWidth ) ) { endNode = openTiles[openTiles.size() - 1]; break; }
		if ( OpenAdjacentTile( openTiles, eCardinalDirections::NORTH, nodeToProcess, endTile, tileTypeCosts, observedTileTypes, mapWidth ) ) { endNode = openTiles[openTiles.size() - 1]; break; }
		if ( OpenAdjacentTile( openTiles, eCardinalDirections::SOUTH, nodeToProcess, endTile, tileTypeCosts, observedTileTypes, mapWidth ) ) { endNode = openTiles[openTiles.size() - 1]; break; }

		nodeToProcess.hasBeenProcessed = true;
	}

	int nextNodeIdx = endNode.idx;
	while ( nextNodeIdx != -1 
			&& nextNodeIdx < openTiles.size() )
	{
		PathNode const& nextNode = openTiles[nextNodeIdx];
		pathToGoal.push_back( nextNode.tileCoords );
		nextNodeIdx = nextNode.fromNodeIdx;
	}

	return pathToGoal;
}


//-----------------------------------------------------------------------------------------------
int PathGenerator::GetBestOpenNode( std::vector<PathNode>& openTiles )
{
	// Don't call this with an empty list

	if ( openTiles.size() == 1 )
	{
		return 0;
	}

	// TODO: What if last node has been processed?
	int bestTileIdx = 0;
	int lowestFCost = 9999999;
	int lowestHCost = 9999999;

	for ( int tileIdx = 0; tileIdx < (int)openTiles.size(); ++tileIdx )
	{
		if ( openTiles[tileIdx].hasBeenProcessed )
		{
			continue;
		}

		if ( openTiles[tileIdx].GetFCost() < lowestFCost )
		{
			bestTileIdx = tileIdx;
			lowestFCost = openTiles[tileIdx].GetFCost();
			lowestHCost = openTiles[tileIdx].h;
		}
		else if ( openTiles[tileIdx].GetFCost() == lowestFCost
				  && openTiles[tileIdx].h < lowestHCost )
		{
			bestTileIdx = tileIdx;
			lowestFCost = openTiles[tileIdx].GetFCost();
			lowestHCost = openTiles[tileIdx].h;
		}
	}

	// TODO: Make sure it's never valid to process starting node again
	if ( bestTileIdx == 0 )
	{
		bestTileIdx = -1;
	}

	return bestTileIdx;
}


//-----------------------------------------------------------------------------------------------
bool PathGenerator::OpenAdjacentTile( std::vector<PathNode>& openTiles, eCardinalDirections direction,
							   const PathNode& curTile, const TileCoords& endTileCoords,
							   const int* tileTypeCosts, const eTileType* observedTileTypes, int mapWidth )
{
	int tileIdx = GetAdjacentTileIdx( curTile.tileCoords, direction, mapWidth );
	// Check for tile outside map
	if ( tileIdx == -1 )
	{
		return false;
	}

	TileCoords adjacentTileCoords = GetTileCoordsFromTileIdx( tileIdx, mapWidth );
	TileCoords tileCoordsDiff = endTileCoords - adjacentTileCoords;

	int h = tileCoordsDiff.GetTaxicabLength();

	if ( observedTileTypes[tileIdx] == TILE_TYPE_UNSEEN )
	{
		return false;
	}

	int gPenalty = tileTypeCosts[observedTileTypes[tileIdx]];
	if ( gPenalty == -1 )
	{
		return false;
	}


	eCardinalDirections fromDirection = NONE;
	switch ( direction )
	{
		case NORTH: fromDirection = SOUTH; break;
		case SOUTH: fromDirection = NORTH; break;
		case EAST:	fromDirection = WEST; break;
		case WEST:	fromDirection = EAST; break;
	}

	// Check if we found end
	if ( tileCoordsDiff == TileCoords( 0, 0 ) )
	{
		openTiles.emplace_back( adjacentTileCoords, gPenalty, curTile.gTotal, 0, (int)openTiles.size(), curTile.idx );
		return true;
	}

	// See if we've found this node and update if a shorter path
	for ( int tileIdx = 0; tileIdx < (int)openTiles.size(); ++tileIdx )
	{
		if ( openTiles[tileIdx].tileCoords == adjacentTileCoords )
		{
			if ( gPenalty + curTile.gTotal + h < openTiles[tileIdx].GetFCost() )
			{
				openTiles[tileIdx].gTotal = curTile.gTotal + h;
				if ( curTile.fromNodeIdx != openTiles[tileIdx].idx )
				{
					openTiles[tileIdx].fromNodeIdx = curTile.fromNodeIdx;
				}
			}
			return false;
		}
	}

	openTiles.emplace_back( adjacentTileCoords, gPenalty, curTile.gTotal, h, (int)openTiles.size(), curTile.idx );
	return false;
}

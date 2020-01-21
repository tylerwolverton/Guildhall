#include "Game/MapDefinition.hpp"


//-----------------------------------------------------------------------------------------------
TileWorm::TileWorm( TileType tileType, int numWorms, int wormLength )
	: m_wormTileType( tileType )
	, m_numWorms( numWorms )
	, m_wormLength( wormLength )
{
}


//-----------------------------------------------------------------------------------------------
MapEntityDefinition::MapEntityDefinition( int numNPCTanks, int numNPCTurrets, int numBoulders )
	: m_numNPCTanks( numNPCTanks )
	, m_numNPCTurrets( numNPCTurrets )
	, m_numBoulders( numBoulders )
{
}


//-----------------------------------------------------------------------------------------------
MapDefinition::MapDefinition( IntVec2 dimensions, TileType defaultTileType, TileType edgeTileType, 
							  TileType startTileType, TileType exitTileType, TileType shieldTileType, 
							  std::vector<TileWorm> tileWorms,
							  const MapEntityDefinition& mapEntityDefinition )
	: m_dimensions( dimensions )
	, m_defaultTileType( defaultTileType )
	, m_edgeTileType( edgeTileType )
	, m_startTileType( startTileType )
	, m_exitTileType( exitTileType )
	, m_shieldTileType( shieldTileType )
	, m_tileWorms( tileWorms )
	, m_mapEntityDefinition( mapEntityDefinition )
{
}

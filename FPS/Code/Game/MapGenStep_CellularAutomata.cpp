#include "Game/MapGenStep_CellularAutomata.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
MapGenStep_CellularAutomata::MapGenStep_CellularAutomata( const XmlElement& mapGenStepXmlElem )
	: MapGenStep( mapGenStepXmlElem )
	, m_numNeighbors( 1, 9999999 )
	, m_neighborRadius( 1 )
{
	GUARANTEE_OR_DIE( m_setTileType != nullptr, "No setTile defined. Must have a set tile type to run a cellular automata step." );

	std::string neighborTileName;
	neighborTileName = ParseXmlAttribute( mapGenStepXmlElem, "neighborTile", neighborTileName );
	m_neighborTileType = TileDefinition::GetTileDefinition( neighborTileName );

	m_numNeighbors = ParseXmlAttribute( mapGenStepXmlElem, "numNeighbors", m_numNeighbors );
	m_neighborRadius = ParseXmlAttribute( mapGenStepXmlElem, "neighborRadius", m_neighborRadius );
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_CellularAutomata::RunStepOnce( Map& map )
{
	if ( m_setTileType == nullptr )
	{
		return;
	}

	float chancePerTile = m_chancePerTile.GetRandomInRange( g_game->m_rng );

	// Loop over all tiles and make vector of indexes for tiles to change
	std::vector<Tile*> tilesToChange;

	constexpr int numDirections = sizeof( map.m_cardinalDirectionOffsets ) / sizeof( map.m_cardinalDirectionOffsets[0] );
	for ( int tileIndex = 0; tileIndex < (int)map.m_tiles.size(); ++tileIndex )
	{
		Tile* tile = &map.m_tiles[ tileIndex ];
		
		if ( ( m_ifTileType != nullptr && tile->m_tileDef != m_ifTileType )
			 || !g_game->m_rng->RollPercentChance( chancePerTile ) )
		{
			continue;
		}

		// Count each neighboring tile that will affect the current tile
		int numNeighborsFound = 0;
		std::vector<Tile*> potentialNeighboringTiles = map.GetTilesInRadius( *tile, m_neighborRadius.GetRandomInRange( g_game->m_rng ), false );
		for ( int neighborIndex = 0; neighborIndex < (int)potentialNeighboringTiles.size(); ++neighborIndex )
		{
			if ( potentialNeighboringTiles[ neighborIndex ]->m_tileDef == m_neighborTileType )
			{
				++numNeighborsFound;
			}
		}
		
		if ( m_numNeighbors.IsInRange( numNeighborsFound ) )
		{
			tilesToChange.push_back( tile );
		}
	}

	// Change any affected tiles
	for ( int tileToChangeIndex = 0; tileToChangeIndex < (int)tilesToChange.size(); ++tileToChangeIndex )
	{
		tilesToChange[ tileToChangeIndex ]->SetTileDef( m_setTileType );
	}
}

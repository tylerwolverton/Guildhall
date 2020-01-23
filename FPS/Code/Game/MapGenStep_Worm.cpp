#include "Game/MapGenStep_Worm.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
MapGenStep_Worm::MapGenStep_Worm( const XmlElement& mapGenStepXmlElem )
	: MapGenStep( mapGenStepXmlElem )
	, m_numWorms( 5, 10 )
	, m_wormLength( 3, 8 )
{
	GUARANTEE_OR_DIE( m_setTileType != nullptr, "No setTile defined. Must have a set tile type to run a worm step." );

	m_numWorms = ParseXmlAttribute( mapGenStepXmlElem, "numWorms", m_numWorms );
	m_wormLength = ParseXmlAttribute( mapGenStepXmlElem, "wormLength", m_wormLength );
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_Worm::RunStepOnce( Map& map )
{
	if ( m_setTileType == nullptr )
	{
		return;
	}

	float chancePerTile = m_chancePerTile.GetRandomInRange( g_game->m_rng );
	
	int numWorms = m_numWorms.GetRandomInRange( g_game->m_rng );
	for ( int wormNum = 0; wormNum < numWorms; ++wormNum )
	{
		IntVec2 nextPosition = IntVec2( g_game->m_rng->RollRandomIntLessThan( map.m_width ),
									    g_game->m_rng->RollRandomIntLessThan( map.m_height ) );

		int wormLength = m_wormLength.GetRandomInRange( g_game->m_rng );
		for ( int wormSegmentNum = 0; wormSegmentNum < wormLength; ++wormSegmentNum )
		{
			int tileIndex = map.GetTileIndexFromTileCoords( nextPosition );
			if ( tileIndex != -1 )
			{
				Tile& tile = map.m_tiles[ tileIndex ];

				if ( ( m_ifTileType == nullptr || tile.m_tileDef == m_ifTileType )
						&& g_game->m_rng->RollPercentChance( chancePerTile ) )
				{
					tile.SetTileDef( m_setTileType );
				}
			}

			nextPosition += RollRandomCardinalDirection2D();
		}
	}
}


//-----------------------------------------------------------------------------------------------
IntVec2 MapGenStep_Worm::RollRandomCardinalDirection2D()
{
	static IntVec2 cardinalDirections[4] =
	{
		IntVec2( -1, 0 ),
		IntVec2( 1, 0 ),
		IntVec2( 0, 1 ),
		IntVec2( 0, -1 )
	};

	int directionIndex = g_game->m_rng->RollRandomIntInRange( 0, 3 );
	return cardinalDirections[directionIndex];
}

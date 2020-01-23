#include "Game/MapGenStep_Mutate.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
MapGenStep_Mutate::MapGenStep_Mutate( const XmlElement& mapGenStepXmlElem )
	: MapGenStep( mapGenStepXmlElem )
{
	GUARANTEE_OR_DIE( m_setTileType != nullptr, "No setTile defined. Must have a set tile type to run a mutate step." );
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_Mutate::RunStepOnce( Map& map )
{
	if ( m_setTileType == nullptr )
	{
		return;
	}

	float chancePerTile = m_chancePerTile.GetRandomInRange( g_game->m_rng );
	for(int tileIndex = 0; tileIndex < (int)map.m_tiles.size(); ++tileIndex)
	{
		if( !g_game->m_rng->RollPercentChance( chancePerTile ) )
		{
			continue;
		}

		Tile& tile = map.m_tiles[ tileIndex ];
		if( !m_ifTileType 
			|| tile.m_tileDef == m_ifTileType )
		{
			tile.SetTileDef( m_setTileType );
		}
	}
}

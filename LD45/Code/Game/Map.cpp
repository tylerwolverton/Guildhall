#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( int width, int height )
	: m_width( width )
	, m_height( height )
{
	for ( int yIndex = 0; yIndex < m_height; ++yIndex )
	{
		for ( int xIndex = 0; xIndex < m_width; ++xIndex )
		{
			m_tiles.push_back( Tile( IntVec2( xIndex * TILE_SIZE, yIndex * TILE_SIZE ) ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{

}


//-----------------------------------------------------------------------------------------------
void Map::Render() const
{
	for ( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		m_tiles[tileIndex].Render();
	}
}

#include "Game/Tile.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Tile::Tile( IntVec2 tileCoords, TileType tileType )
	: m_tileCoords( tileCoords )
	, m_tileType( tileType )
{
}


//-----------------------------------------------------------------------------------------------
Tile::Tile( int x, int y, TileType tileType )
	: m_tileCoords( IntVec2(x, y) )
	, m_tileType( tileType )
{
}


//-----------------------------------------------------------------------------------------------
Tile::~Tile()
{
}


//-----------------------------------------------------------------------------------------------
AABB2 Tile::GetBounds() const
{
	float tileX = (float)m_tileCoords.x;
	float tileY = (float)m_tileCoords.y;

	Vec2 bottomLeft( tileX, tileY );
	Vec2 topRight( tileX + TILE_SIZE, tileY + TILE_SIZE );

	return AABB2( bottomLeft, topRight );
}


//-----------------------------------------------------------------------------------------------
Rgba8 GetColorForTileType( TileType tileType )
{
	switch ( tileType )
	{
		case TileType::TILE_TYPE_GRASS:
			return Rgba8( 0, 127, 0 );
		case TileType::TILE_TYPE_STONE:
			return Rgba8( 180, 180, 180 );
	}

	return Rgba8( 255, 255, 255 );
}


//-----------------------------------------------------------------------------------------------
bool IsTileTypeSolid( TileType tileType )
{
	switch ( tileType )
	{
		case TileType::TILE_TYPE_GRASS:
			return false;
		case TileType::TILE_TYPE_STONE:
			return true;
	}

	return false;
}

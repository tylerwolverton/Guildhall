#include "Game/Tile.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"


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
	if ( tileType == TILE_TYPE_INVALID )
	{
		return Rgba8( 127, 0, 0 );
	}

	return TileDefinition::s_definitions[tileType].GetTintColor();
}


//-----------------------------------------------------------------------------------------------
bool IsTileTypeSolid( TileType tileType )
{
	if ( tileType == TILE_TYPE_INVALID )
	{
		return false;
	}

	return TileDefinition::s_definitions[tileType].IsSolid();
}


//-----------------------------------------------------------------------------------------------
bool DoesTileTypeStopBullets( TileType tileType )
{
	if ( tileType == TILE_TYPE_INVALID )
	{
		return true;
	}

	return TileDefinition::s_definitions[tileType].StopsBullets();
}


//-----------------------------------------------------------------------------------------------
AABB2 GetUVsForTileType( TileType tileType )
{
	return TileDefinition::s_definitions[tileType].GetUVCoords();
}

#include "Game/Tile.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Tile::Tile( IntVec2 tileCoords, TileDefinition* tileDef )
	: m_tileCoords( tileCoords )
	, m_tileDef( tileDef )
{
}


//-----------------------------------------------------------------------------------------------
Tile::Tile( int x, int y, TileDefinition* tileDef )
	: m_tileCoords( IntVec2(x, y) )
	, m_tileDef( tileDef )
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
bool Tile::AllowsWalking() const
{
	return m_tileDef->m_allowsWalking;
}


//-----------------------------------------------------------------------------------------------
bool Tile::AllowsSwimming() const
{
	return m_tileDef->m_allowsSwimming;
}


//-----------------------------------------------------------------------------------------------
bool Tile::AllowsFlying() const
{
	return m_tileDef->m_allowsFlying;
}

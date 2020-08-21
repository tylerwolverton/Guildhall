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
std::string Tile::GetName() const
{
	if ( m_tileDef == nullptr )
	{
		return "";
	}

	return m_tileDef->GetName();
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
bool Tile::IsSolid() const
{
	if ( m_tileDef == nullptr )
	{
		return true;
	}

	return m_tileDef->m_isSolid;
}

//-----------------------------------------------------------------------------------------------
TileMaterialDefinition* Tile::GetTileMaterialDef() const
{
	if ( m_tileDef == nullptr )
	{
		return nullptr;
	}

	return m_tileDef->m_matDef;
}

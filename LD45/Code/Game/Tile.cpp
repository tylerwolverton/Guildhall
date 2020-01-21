#include "Game/Tile.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Tile::Tile( IntVec2 tileCoords )
	: m_tileCoords( tileCoords )
{
}

//-----------------------------------------------------------------------------------------------
Tile::~Tile()
{
}

//-----------------------------------------------------------------------------------------------
void Tile::Render() const
{
	Vec2 bottomLeft( (float)m_tileCoords.x, (float)m_tileCoords.y );
	Vec2 topRight( (float)( m_tileCoords.x + TILE_SIZE ), (float)( m_tileCoords.y + TILE_SIZE ) );
	AABB2 box(bottomLeft, topRight);
	
	g_theRenderer->DrawAABB2( box, Rgba8(0, 0, 255), .2f);
}

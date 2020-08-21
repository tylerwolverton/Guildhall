#include "Game/TileMap.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/MapData.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/TileMaterialDefinition.hpp"


//-----------------------------------------------------------------------------------------------
TileMap::TileMap( const MapData& mapData )
	: Map( mapData )
{
	m_dimensions = mapData.dimensions;

	BuildCardinalDirectionsArray();
	PopulateTiles( mapData.tileDefs );
}


//-----------------------------------------------------------------------------------------------
TileMap::~TileMap()
{
}


//-----------------------------------------------------------------------------------------------
void TileMap::Load()
{
}


//-----------------------------------------------------------------------------------------------
void TileMap::Unload()
{

}


//-----------------------------------------------------------------------------------------------
void TileMap::Update( float deltaSeconds )
{
	Map::Update( deltaSeconds );

	ResolveEntityVsWallCollisions();
}


//-----------------------------------------------------------------------------------------------
void TileMap::UpdateMesh()
{
	m_mesh.clear();

	for ( int tileIdx = 0; tileIdx < (int)m_tiles.size(); ++tileIdx )
	{
		Tile& tile = m_tiles[tileIdx];
		
		TileMaterialDefinition* materialTypeDef = tile.GetTileMaterialDef();
		if ( materialTypeDef == nullptr )
		{
			return;
		}

		Vec2 bottomLeft( tile.GetBounds().mins );
		Vec2 bottomRight( tile.GetBounds().maxs.x, tile.GetBounds().mins.y );
		Vec2 topLeft( tile.GetBounds().mins.x, tile.GetBounds().maxs.y );
		Vec2 topRight( tile.GetBounds().maxs );

		Vec2 uvsAtMins = materialTypeDef->GetUVCoords().mins;
		Vec2 uvsAtMaxs = materialTypeDef->GetUVCoords().maxs;

		m_mesh.push_back( Vertex_PCU( bottomLeft, Rgba8::WHITE, uvsAtMins ) );
		m_mesh.push_back( Vertex_PCU( bottomRight, Rgba8::WHITE, Vec2( uvsAtMaxs.x, uvsAtMins.y ) ) );
		m_mesh.push_back( Vertex_PCU( topRight, Rgba8::WHITE, uvsAtMaxs ) );

		m_mesh.push_back( Vertex_PCU( bottomLeft, Rgba8::WHITE, uvsAtMins ) );
		m_mesh.push_back( Vertex_PCU( topRight, Rgba8::WHITE, uvsAtMaxs ) );
		m_mesh.push_back( Vertex_PCU( topLeft, Rgba8::WHITE, Vec2( uvsAtMins.x, uvsAtMaxs.y ) ) );
	}
}


//-----------------------------------------------------------------------------------------------
void TileMap::Render() const
{
	Map::Render();

	if ( m_mesh.size() == 0 )
	{
		return;
	}

	g_renderer->SetModelMatrix( Mat44::IDENTITY );
	g_renderer->SetBlendMode( eBlendMode::ALPHA );
	g_renderer->BindShaderProgram( g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/Default.hlsl" ) );

	g_renderer->BindTexture( 0, &( m_tiles[0].GetTileMaterialDef()->GetSpriteSheet()->GetTexture() ) );

	g_renderer->DrawVertexArray( m_mesh );
}


//-----------------------------------------------------------------------------------------------
void TileMap::DebugRender() const
{
	Map::DebugRender();

	for ( int tileIdx = 0; tileIdx < (int)m_tiles.size(); ++tileIdx )
	{
		const Tile& tile = m_tiles[tileIdx];

		g_renderer->BindDiffuseTexture( nullptr );
		DrawAABB2Outline( g_renderer, tile.GetBounds(), Rgba8::CYAN, 2.f );
	}
}


//-----------------------------------------------------------------------------------------------
RaycastResult TileMap::Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const
{
	RaycastResult closestImpact;
	closestImpact.impactDist = maxDist;

	RaycastResult wallsResult = RaycastAgainstWalls( startPos, forwardNormal, maxDist );
	if ( wallsResult.didImpact
		 && wallsResult.impactDist < closestImpact.impactDist )
	{
		closestImpact = wallsResult;
	}
	
	return closestImpact;
}


//-----------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstWalls( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) const
{
	RaycastResult result;
	result.startPos = startPos;
	result.forwardNormal = forwardNormal;
	result.maxDist = maxDist;

	// Check if starting tile is solid
	const Tile* startTile = GetTileFromWorldCoords( startPos );
	if ( startTile == nullptr 
		 || startTile->IsSolid() )
	{
		result.didImpact = true;
		result.impactFraction = 0.f;
		result.impactDist = 0.f;
		result.impactPos = startPos;
		result.impactSurfaceNormal = -forwardNormal;

		return result;
	}

	// Calculate set up values to be used in each step of the raycast
	Vec2 rayDisp = forwardNormal * maxDist;

	// How far along the ray do you have to go to move 1 unit in the x direction of the grid
	// if ray doesn't have any x movement this value is essentially infinity
	float xDeltaDistAlongRay = 99999999.f;
	if ( !IsNearlyEqual( rayDisp.x, 0.f, .000001f ) )
	{
		xDeltaDistAlongRay = maxDist / fabs( rayDisp.x );
	}
	// +1 or -1 to indicate which direction the steps will take
	int tileStepDirX = (int)SignFloat( rayDisp.x );
	// Instead of starting ray in the middle of a tile, adjust the start position
	// to one of the edges of the starting tile depending on which way the ray faces
	int offsetInTileCoordsToLeadingEdgeX = ( tileStepDirX + 1 ) / 2;
	float firstVerticalIntersectionX = (float)( startTile->m_tileCoords.x + offsetInTileCoordsToLeadingEdgeX );

	float dOfNextXCrossing = fabs( firstVerticalIntersectionX - startPos.x ) * xDeltaDistAlongRay;

	// Repeat everything above but for y
	float yDeltaDistAlongRay = 99999999.f;
	if ( !IsNearlyEqual( rayDisp.y, 0.f, .000001f ) )
	{
		yDeltaDistAlongRay = maxDist / fabs( rayDisp.y );
	}
	int tileStepDirY = (int)SignFloat( rayDisp.y );

	int offsetInTileCoordsToLeadingEdgeY = ( tileStepDirY + 1 ) / 2;
	float firstHorizontalIntersectionY = (float)( startTile->m_tileCoords.y + offsetInTileCoordsToLeadingEdgeY );

	float dOfNextYCrossing = fabs( firstHorizontalIntersectionY - startPos.y ) * yDeltaDistAlongRay;

	int tileCoordX = startTile->m_tileCoords.x;
	int tileCoordY = startTile->m_tileCoords.y;

	// Perform raycast
	while ( dOfNextXCrossing <= maxDist
			|| dOfNextYCrossing <= maxDist )
	{
		// We'll cross X line next
		if ( dOfNextXCrossing < dOfNextYCrossing )
		{
			tileCoordX += tileStepDirX;
			// Hit a solid tile
			if ( IsTileSolid( tileCoordX, tileCoordY ) )
			{
				result.didImpact = true;
				result.impactFraction = dOfNextXCrossing / maxDist;
				Vec2 startToImpact( forwardNormal * dOfNextXCrossing );
				result.impactPos = startPos + startToImpact;
				result.impactDist = dOfNextXCrossing;
				result.impactSurfaceNormal = Vec2( (float)-tileStepDirX, 0.f );
				break;
			}

			dOfNextXCrossing += xDeltaDistAlongRay;
		}
		// We'll cross Y line next
		else
		{
			tileCoordY += tileStepDirY;
			// Hit a solid tile
			if ( IsTileSolid( tileCoordX, tileCoordY ) )
			{
				result.didImpact = true;
				result.impactFraction = dOfNextYCrossing/ maxDist;
				Vec2 startToImpact( forwardNormal * dOfNextYCrossing );
				result.impactPos = startPos + startToImpact;
				result.impactDist = dOfNextYCrossing;
				result.impactSurfaceNormal = Vec2( 0.f, (float)-tileStepDirY );
				break;
			}

			dOfNextYCrossing += yDeltaDistAlongRay;
		}
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void TileMap::PopulateTiles( const std::vector<TileDefinition*>& tileDefs )
{
	CreateInitialTiles( tileDefs );
	//SolidifySurroundingTiles();
}


//-----------------------------------------------------------------------------------------------
void TileMap::CreateInitialTiles( const std::vector<TileDefinition*>& tileDefs )
{
	for ( int y = 0; y < m_dimensions.y; ++y )
	{
		for ( int x = 0; x < m_dimensions.x; ++x )
		{
			m_tiles.push_back( Tile( IntVec2( x, y ), tileDefs[( y * m_dimensions.x ) + x] ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void TileMap::SolidifySurroundingTiles()
{
	for ( int y = 0; y < m_dimensions.y; ++y )
	{
		for ( int x = 0; x < m_dimensions.x; ++x )
		{
			if ( x == 0 || x == m_dimensions.x - 1
				|| y == 0 || y == m_dimensions.y - 1 )
			{
				//GetTileFromTileCoords( IntVec2( x, y ) )->m_isSolid = true;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void TileMap::SpawnPlayer()
{

}


//-----------------------------------------------------------------------------------------------
void TileMap::RenderTiles() const
{
	std::vector<Vertex_PCU> vertices;

	for ( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		//const Tile& tile = m_tiles[tileIndex];

		//AppendVertsForAABB2D( vertices, tile.GetBounds(), tile.m_tileDef->GetSpriteTint(), tile.m_tileDef->GetUVCoords().mins, tile.m_tileDef->GetUVCoords().maxs );
	}

	//g_renderer->BindTexture( &(g_tileSpriteSheet->GetTexture()) );
	//g_renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void TileMap::BuildCardinalDirectionsArray()
{
	// 5 1 6
	// 4 * 2
	// 8 3 7
	m_cardinalDirectionOffsets[(int)eCardinalDirection::CENTER] = Vec2::ZERO;
	m_cardinalDirectionOffsets[(int)eCardinalDirection::NORTH] = Vec2( 0.f, TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirection::EAST] = Vec2( TILE_SIZE, 0.f );
	m_cardinalDirectionOffsets[(int)eCardinalDirection::SOUTH] = Vec2( 0.f, -TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirection::WEST] = Vec2( -TILE_SIZE, 0.f );
	m_cardinalDirectionOffsets[(int)eCardinalDirection::NORTHWEST] = Vec2( -TILE_SIZE, TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirection::NORTHEAST] = Vec2( TILE_SIZE, TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirection::SOUTHEAST] = Vec2( TILE_SIZE, -TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirection::SOUTHWEST] = Vec2( -TILE_SIZE, -TILE_SIZE );
}


//-----------------------------------------------------------------------------------------------
void TileMap::ResolveEntityVsWallCollisions()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		ResolveEntityVsWallCollision( *entity );
	}
}


//-----------------------------------------------------------------------------------------------
void TileMap::ResolveEntityVsWallCollision( Entity& entity )
{
	if ( !entity.m_canBePushedByWalls )
	{
		return;
	}

	const Tile* entityTile = GetTileFromWorldCoords( entity.GetPosition() );
	if ( entityTile == nullptr )
	{
		return;
	}

	std::vector<const Tile*> surroundingTiles = GetTilesInRadius( *entityTile, 1, true );
	for ( int tileIdx = 0; tileIdx < (int)surroundingTiles.size(); ++tileIdx )
	{
		const Tile*& tile = surroundingTiles[tileIdx];
		if ( tile != nullptr
			 && tile->IsSolid() )
		{
			PushDiscOutOfAABB2D( entity.m_position, entity.GetPhysicsRadius(), tile->GetBounds() );
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool TileMap::IsAdjacentTileSolid( const Tile& tile, eCardinalDirection direction ) const
{
	const Tile* adjacentTile = GetTileFromWorldCoords( Vec2( tile.m_tileCoords ) + m_cardinalDirectionOffsets[(int)direction] );

	if ( adjacentTile == nullptr
		 || adjacentTile->m_tileDef == nullptr )
	{
		return true;
	}

	return adjacentTile->IsSolid();
}


//-----------------------------------------------------------------------------------------------
bool TileMap::IsTileSolid( int xCoord, int yCoord ) const
{
	const Tile* tile = GetTileFromTileCoords( xCoord, yCoord );

	if ( tile == nullptr
		 || tile->m_tileDef == nullptr )
	{
		return true;
	}

	return tile->IsSolid();
}


//-----------------------------------------------------------------------------------------------
int TileMap::GetTileIndexFromTileCoords( int xCoord, int yCoord ) const
{
	if ( xCoord < 0
		 || xCoord > m_dimensions.x - 1
		 || yCoord < 0
		 || yCoord > m_dimensions.y - 1 )
	{
		return -1;
	}

	return xCoord + yCoord * m_dimensions.x;
}

//-----------------------------------------------------------------------------------------------
int TileMap::GetTileIndexFromTileCoords( const IntVec2& coords ) const
{
	return GetTileIndexFromTileCoords( coords.x, coords.y );
}


//-----------------------------------------------------------------------------------------------
int TileMap::GetTileIndexFromWorldCoords( const Vec2& coords ) const
{
	return GetTileIndexFromTileCoords( (int)coords.x, (int)coords.y );
}


//-----------------------------------------------------------------------------------------------
const Tile* TileMap::GetTileFromTileCoords( const IntVec2& tileCoords ) const
{
	return GetTileFromTileCoords( tileCoords.x, tileCoords.y );
}


//-----------------------------------------------------------------------------------------------
const Tile* TileMap::GetTileFromTileCoords( int xCoord, int yCoord ) const
{
	int tileIndex = GetTileIndexFromTileCoords( xCoord, yCoord );
	if ( tileIndex < 0
		 || tileIndex >= (int)m_tiles.size() )
	{
		return nullptr;
	}

	return &( m_tiles[tileIndex] );
}


//-----------------------------------------------------------------------------------------------
const Tile* TileMap::GetTileFromWorldCoords( const Vec2& worldCoords ) const
{
	return GetTileFromTileCoords( IntVec2( (int)worldCoords.x, (int)worldCoords.y ) );
}


//-----------------------------------------------------------------------------------------------
const Vec2 TileMap::GetWorldCoordsFromTile( const Tile& tile ) const
{
	return Vec2( (float)tile.m_tileCoords.x, (float)tile.m_tileCoords.y );
}


//-----------------------------------------------------------------------------------------------
std::vector<const Tile*> TileMap::GetTilesInRadius( const Tile& centerTile, int radius, bool includeCenterTile ) const
{
	std::vector<const Tile*> surroundingTiles;

	IntVec2 mins( centerTile.m_tileCoords.x - radius, centerTile.m_tileCoords.y - radius );
	int sideLength = ( 2 * radius ) + 1;

	for ( int yPos = 0; yPos < sideLength; ++yPos )
	{
		for ( int xPos = 0; xPos < sideLength; ++xPos )
		{
			IntVec2 tileCoords( mins.x + xPos, mins.y + yPos );

			if ( tileCoords == centerTile.m_tileCoords
				 && !includeCenterTile )
			{
				continue;
			}

			const Tile* tile = GetTileFromTileCoords( tileCoords );
			if ( tile != nullptr )
			{
				surroundingTiles.push_back( tile );
			}
		}
	}

	return surroundingTiles;
}

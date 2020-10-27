#include "Game/TileMap.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
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
TileMap::TileMap( const MapData& mapData, World* world )
	: Map( mapData, world )
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
void TileMap::Load( Entity* player )
{
	Map::Load( player );

	CreateTileRigidbodies();
}


//-----------------------------------------------------------------------------------------------
void TileMap::Unload()
{
	DestroyTileRigidbodies();

	Map::Unload();
}


//-----------------------------------------------------------------------------------------------
void TileMap::Update( float deltaSeconds )
{
	Map::Update( deltaSeconds );

	UpdateCameras();
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
void TileMap::UpdateCameras()
{
	CenterCameraOnPlayer();
}


//-----------------------------------------------------------------------------------------------
void TileMap::CenterCameraOnPlayer() const
{
	if ( m_player != nullptr )
	{
		Vec2 halfWindowSize( WINDOW_WIDTH * .5f, WINDOW_HEIGHT * .5f );
		AABB2 cameraBounds( m_player->GetPosition() - halfWindowSize, m_player->GetPosition() + halfWindowSize );

		AABB2 windowBox( Vec2( 0.f, 0.f ), Vec2( (float)m_dimensions.x, (float)m_dimensions.y ) );
		cameraBounds.FitWithinBounds( windowBox );

		g_game->SetWorldCameraPosition( Vec3( cameraBounds.GetCenter(), 0.f ) );
	}
}


//-----------------------------------------------------------------------------------------------
void TileMap::Render() const
{
	if ( m_mesh.size() == 0 )
	{
		return;
	}

	g_renderer->SetModelMatrix( Mat44::IDENTITY );
	g_renderer->SetBlendMode( eBlendMode::ALPHA );
	g_renderer->BindShaderProgram( g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/Default.hlsl" ) );

	g_renderer->BindTexture( 0, &( m_tiles[0].GetTileMaterialDef()->GetSpriteSheet()->GetTexture() ) );

	g_renderer->DrawVertexArray( m_mesh );

	// Render entities
	Map::Render();
}


//-----------------------------------------------------------------------------------------------
void TileMap::DebugRender() const
{
	Map::DebugRender();

	for ( int tileIdx = 0; tileIdx < (int)m_tiles.size(); ++tileIdx )
	{
		const Tile& tile = m_tiles[tileIdx];

		g_renderer->BindDiffuseTexture( nullptr );
		DrawAABB2Outline( g_renderer, tile.GetBounds(), Rgba8::CYAN, .1f );
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
void TileMap::CreateTileRigidbodies()
{
	m_tileRigidbodies.reserve( m_tiles.size() );

	for ( int tileIdx = 0; tileIdx < (int)m_tiles.size(); ++tileIdx )
	{
		if ( m_tiles[tileIdx].IsSolid() )
		{
			Rigidbody2D* rigidbody2D = g_physicsSystem2D->CreateRigidbody();

			PolygonCollider2D* polygonCollider = g_physicsSystem2D->CreatePolygon2Collider( m_tiles[tileIdx].GetBounds().GetAsPolygon2() );
			rigidbody2D->TakeCollider( polygonCollider );
			rigidbody2D->SetSimulationMode( SIMULATION_MODE_STATIC );
			rigidbody2D->SetPosition( m_tiles[tileIdx].GetBounds().GetCenter() );
			rigidbody2D->SetLayer( eCollisionLayer::STATIC_ENVIRONMENT );

			m_tileRigidbodies.push_back( rigidbody2D );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void TileMap::DestroyTileRigidbodies()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_tileRigidbodies.size(); ++rigidbodyIdx )
	{
		m_tileRigidbodies[rigidbodyIdx]->Destroy();
	}

	m_tileRigidbodies.clear();
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

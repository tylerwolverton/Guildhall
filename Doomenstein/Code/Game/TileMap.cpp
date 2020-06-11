#include "Game/TileMap.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/MapRegionTypeDefinition.hpp"
#include "Game/MapMaterialTypeDefinition.hpp"


//-----------------------------------------------------------------------------------------------
TileMap::TileMap( std::string name, MapDefinition* mapDef )
	: Map( name, mapDef )
{
	BuildCardinalDirectionsArray();
	PopulateTiles();
	CreateTestBoxes();

	g_game->SetCameraPositionAndYaw( mapDef->GetPlayerStartPos(), mapDef->GetPlayerStartYaw() );
}


//-----------------------------------------------------------------------------------------------
TileMap::~TileMap()
{
	PTR_SAFE_DELETE( m_cubeMesh );
	PTR_SAFE_DELETE( m_testMaterial );
}


//-----------------------------------------------------------------------------------------------
void TileMap::UpdateMeshes()
{
	m_mesh.clear();

	for ( int tileIdx = 0; tileIdx < (int)m_tiles.size(); ++tileIdx )
	{
		Tile& tile = m_tiles[tileIdx];

		Vec2 mins( (float)tile.m_tileCoords.x, (float)tile.m_tileCoords.y );
		Vec2 maxs( mins + Vec2( TILE_SIZE, TILE_SIZE ) );

		Vec3 vert0( mins, 0.f );
		Vec3 vert1( maxs.x, mins.y, 0.f );
		Vec3 vert2( mins.x, maxs.y, 0.f );
		Vec3 vert3( maxs, 0.f );

		Vec3 vert4( mins, TILE_SIZE );
		Vec3 vert5( maxs.x, mins.y, TILE_SIZE );
		Vec3 vert6( mins.x, maxs.y, TILE_SIZE );
		Vec3 vert7( maxs, TILE_SIZE );

		if ( !tile.IsSolid() )
		{
			Vec2 uvsAtMins, uvsAtMaxs;
			MapMaterialTypeDefinition* materialTypeDef = tile.m_regionTypeDef->GetFloorMaterial();
			materialTypeDef->GetSpriteSheet()->GetSpriteUVs( uvsAtMins, uvsAtMaxs, materialTypeDef->GetSpriteCoords() );
			//materialTypeDef->GetSpriteSheet()->GetSpriteUVs( &uvsAtMins, &uvsAtMaxs, materialTypeDef->GetSpriteCoords() );
			// Bottom face
			AddTileFace( vert0, vert1, vert2, vert3, uvsAtMins, uvsAtMaxs );

			materialTypeDef = tile.m_regionTypeDef->GetCeilingMaterial();
			materialTypeDef->GetSpriteSheet()->GetSpriteUVs( uvsAtMins, uvsAtMaxs, materialTypeDef->GetSpriteCoords() );
			// Top face
			AddTileFace( vert5, vert4, vert7, vert6, uvsAtMins, uvsAtMaxs );
		}
		else
		{
			Vec2 uvsAtMins, uvsAtMaxs;
			MapMaterialTypeDefinition* materialTypeDef = tile.m_regionTypeDef->GetSideMaterial();
			materialTypeDef->GetSpriteSheet()->GetSpriteUVs( uvsAtMins, uvsAtMaxs, materialTypeDef->GetSpriteCoords() );

			// South face
			if ( !IsAdjacentTileSolid( tile, eCardinalDirection::SOUTH ) )
			{
				AddTileFace( vert0, vert1, vert4, vert5, uvsAtMins, uvsAtMaxs );
			}

			// East face
			if ( !IsAdjacentTileSolid( tile, eCardinalDirection::EAST ) )
			{
				AddTileFace( vert1, vert3, vert5, vert7, uvsAtMins, uvsAtMaxs );
			}

			// North face
			if ( !IsAdjacentTileSolid( tile, eCardinalDirection::NORTH ) )
			{
				AddTileFace( vert3, vert2, vert7, vert6, uvsAtMins, uvsAtMaxs );
			}

			// West face
			if ( !IsAdjacentTileSolid( tile, eCardinalDirection::WEST ) )
			{
				AddTileFace( vert2, vert0, vert6, vert4, uvsAtMins, uvsAtMaxs );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void TileMap::AddTileFace( const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topLeft, const Vec3& topRight, const Vec2& uvMins, const Vec2& uvMaxs )
{
	m_mesh.push_back( Vertex_PCU( bottomLeft, Rgba8::WHITE, uvMins ) );
	m_mesh.push_back( Vertex_PCU( bottomRight, Rgba8::WHITE, Vec2( uvMaxs.x, uvMins.y ) ) );
	m_mesh.push_back( Vertex_PCU( topRight, Rgba8::WHITE, uvMaxs ) );

	m_mesh.push_back( Vertex_PCU( bottomLeft, Rgba8::WHITE, uvMins ) );
	m_mesh.push_back( Vertex_PCU( topRight, Rgba8::WHITE, uvMaxs ) );
	m_mesh.push_back( Vertex_PCU( topLeft, Rgba8::WHITE, Vec2( uvMins.x, uvMaxs.y ) ) );
}


//-----------------------------------------------------------------------------------------------
void TileMap::Render() const
{
	//RenderTiles();
	//RenderTestBoxes();

	g_renderer->SetModelMatrix( Mat44::IDENTITY );
	g_renderer->SetBlendMode( eBlendMode::ALPHA );
	g_renderer->BindShaderProgram( g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/Default.hlsl" ) );
	g_renderer->BindTexture( 0, &m_tiles[0].m_regionTypeDef->GetSideMaterial()->GetSpriteSheet()->GetTexture() );
	//g_renderer->BindTexture( 0, g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
	g_renderer->DrawVertexArray( m_mesh );
}


//-----------------------------------------------------------------------------------------------
void TileMap::DebugRender() const
{
}


//-----------------------------------------------------------------------------------------------
void TileMap::PopulateTiles()
{
	CreateInitialTiles();
	//SolidifySurroundingTiles();
}


//-----------------------------------------------------------------------------------------------
void TileMap::CreateInitialTiles()
{
	for ( int y = 0; y < m_dimensions.y; ++y )
	{
		for ( int x = 0; x < m_dimensions.x; ++x )
		{
			m_tiles.push_back( Tile( IntVec2( x, y ), m_mapDef->m_regionTypeDefs[( y * m_dimensions.x ) + x] ) );
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
bool TileMap::IsAdjacentTileSolid( const Tile& tile, eCardinalDirection direction )
{
	Tile* adjacentTile = GetTileFromWorldCoords( Vec2( tile.m_tileCoords ) + m_cardinalDirectionOffsets[(int)direction] );

	if ( adjacentTile == nullptr )
	{
		return true;
	}

	return adjacentTile->IsSolid();
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
void TileMap::CreateTestBoxes()
{
	// Cube
	/*std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;
	AppendVertsAndIndicesForCubeMesh( vertices, indices, Vec3::ZERO, 1.f, Rgba8::WHITE );

	m_cubeMesh = new GPUMesh( g_renderer, vertices, indices );

	Transform cubeTransform;
	cubeTransform.SetPosition( Vec3( 2.5f, 0.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );

	cubeTransform.SetPosition( Vec3( 2.5f, 2.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );

	cubeTransform.SetPosition( Vec3( 0.5f, 2.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );*/

	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	AppendVertsAndIndicesForCubeMesh( vertices, indices, Vec3::ZERO, 1.f, Rgba8::WHITE );

	m_cubeMesh = new GPUMesh( g_renderer, vertices, indices );

	Transform cubeTransform;
	cubeTransform.SetPosition( Vec3( 2.5f, 0.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );

	cubeTransform.SetPosition( Vec3( 2.5f, 2.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );

	cubeTransform.SetPosition( Vec3( 0.5f, 2.5f, 0.5f ) );
	m_cubeMeshTransforms.push_back( cubeTransform );

	// Initialize materials
	m_testMaterial = new Material( g_renderer, "Data/Materials/Test.material" );
}


//-----------------------------------------------------------------------------------------------
void TileMap::RenderTestBoxes() const
{
	// Render normal objects
	for ( int cubeMeshTransformIdx = 0; cubeMeshTransformIdx < (int)m_cubeMeshTransforms.size(); ++cubeMeshTransformIdx )
	{
		Mat44 modelMatrix = m_cubeMeshTransforms[cubeMeshTransformIdx].GetAsMatrix();
		g_renderer->SetModelMatrix( modelMatrix );
		//g_renderer->BindTexture( 0, g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
		g_renderer->BindMaterial( m_testMaterial );
		g_renderer->DrawMesh( m_cubeMesh );
	}
}


//-----------------------------------------------------------------------------------------------
int TileMap::GetTileIndexFromTileCoords( int xCoord, int yCoord )
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
int TileMap::GetTileIndexFromTileCoords( const IntVec2& coords )
{
	return GetTileIndexFromTileCoords( coords.x, coords.y );
}


//-----------------------------------------------------------------------------------------------
int TileMap::GetTileIndexFromWorldCoords( const Vec2& coords )
{
	return GetTileIndexFromTileCoords( (int)coords.x, (int)coords.y );
}


//-----------------------------------------------------------------------------------------------
Tile* TileMap::GetTileFromTileCoords( const IntVec2& tileCoords )
{
	return GetTileFromTileCoords( tileCoords.x, tileCoords.y );
}


//-----------------------------------------------------------------------------------------------
Tile* TileMap::GetTileFromTileCoords( int xCoord, int yCoord )
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
Tile* TileMap::GetTileFromWorldCoords( const Vec2& worldCoords )
{
	return GetTileFromTileCoords( IntVec2( (int)worldCoords.x, (int)worldCoords.y ) );
}


//-----------------------------------------------------------------------------------------------
const Vec2 TileMap::GetWorldCoordsFromTile( const Tile& tile )
{
	return Vec2( (float)tile.m_tileCoords.x, (float)tile.m_tileCoords.y );
}


//-----------------------------------------------------------------------------------------------
std::vector<Tile*> TileMap::GetTilesInRadius( const Tile& centerTile, int radius, bool includeCenterTile )
{
	std::vector<Tile*> surroundingTiles;

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

			Tile* tile = GetTileFromTileCoords( tileCoords );
			if ( tile != nullptr )
			{
				surroundingTiles.push_back( tile );
			}
		}
	}

	return surroundingTiles;
}

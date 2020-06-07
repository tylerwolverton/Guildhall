#include "Game/TileMap.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/MapDefinition.hpp"


//-----------------------------------------------------------------------------------------------
TileMap::TileMap( std::string name, MapDefinition* mapDef )
	: Map( name, mapDef )
{
	m_width = 8;
	m_height = 12;
	PopulateTiles();
	//CreateTestBoxes();
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

		Vec2 mins( tile.m_tileCoords.x, tile.m_tileCoords.y );
		Vec2 maxs( mins + Vec2( TILE_SIZE, TILE_SIZE ) );
		
		Vec3 vert0( mins, 0.f );
		Vec3 vert1( maxs.x, mins.y, 0.f );
		Vec3 vert2( mins.x, maxs.y, 0.f );
		Vec3 vert3( maxs, 0.f );

		m_mesh.push_back( Vertex_PCU( vert0, Rgba8::WHITE, Vec2::ZERO ) );
		m_mesh.push_back( Vertex_PCU( vert1, Rgba8::WHITE, Vec2( 1.f, 0.f ) ) );
		m_mesh.push_back( Vertex_PCU( vert3, Rgba8::WHITE, Vec2::ONE ) );

		m_mesh.push_back( Vertex_PCU( vert0, Rgba8::WHITE, Vec2::ZERO ) );
		m_mesh.push_back( Vertex_PCU( vert3, Rgba8::WHITE, Vec2::ONE ) );
		m_mesh.push_back( Vertex_PCU( vert2, Rgba8::WHITE, Vec2( 0.f, 1.f ) ) );
	}	
}

//-----------------------------------------------------------------------------------------------
void TileMap::Render() const
{
	//RenderTiles();
	//RenderTestBoxes();

	g_renderer->SetModelMatrix( Mat44::IDENTITY );
	g_renderer->BindTexture( 0, g_renderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
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
}


//-----------------------------------------------------------------------------------------------
void TileMap::CreateInitialTiles()
{
	for ( int y = 0; y < m_height; ++y )
	{
		for ( int x = 0; x < m_width; ++x )
		{
			m_tiles.push_back( Tile( IntVec2( x, y ), g_game->m_rng->RollPercentChance( .1f ) ) );
			//m_tiles.push_back( Tile( x, y, m_mapDef->m_fillTile ) );
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
		const Tile& tile = m_tiles[tileIndex];

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
	m_cardinalDirectionOffsets[(int)eCardinalDirections::CENTER] = Vec2::ZERO;
	m_cardinalDirectionOffsets[(int)eCardinalDirections::NORTH] = Vec2( 0.f, TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirections::EAST] = Vec2( TILE_SIZE, 0.f );
	m_cardinalDirectionOffsets[(int)eCardinalDirections::SOUTH] = Vec2( 0.f, -TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirections::WEST] = Vec2( -TILE_SIZE, 0.f );
	m_cardinalDirectionOffsets[(int)eCardinalDirections::NORTHWEST] = Vec2( -TILE_SIZE, TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirections::NORTHEAST] = Vec2( TILE_SIZE, TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirections::SOUTHEAST] = Vec2( TILE_SIZE, -TILE_SIZE );
	m_cardinalDirectionOffsets[(int)eCardinalDirections::SOUTHWEST] = Vec2( -TILE_SIZE, -TILE_SIZE );
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
		 || xCoord > m_width - 1
		 || yCoord < 0
		 || yCoord > m_height - 1 )
	{
		return -1;
	}

	return xCoord + yCoord * m_width;
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

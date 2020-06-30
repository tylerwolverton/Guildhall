#include "Game/TileMap.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
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
#include "Game/MapDefinition.hpp"
#include "Game/MapRegionTypeDefinition.hpp"
#include "Game/MapMaterialTypeDefinition.hpp"


//-----------------------------------------------------------------------------------------------
TileMap::TileMap( std::string name, MapDefinition* mapDef )
	: Map( name, mapDef )
{
	m_dimensions = mapDef->m_dimensions;

	BuildCardinalDirectionsArray();
	PopulateTiles();
	CreateTestBoxes();
}


//-----------------------------------------------------------------------------------------------
TileMap::~TileMap()
{
	PTR_SAFE_DELETE( m_cubeMesh );
	PTR_SAFE_DELETE( m_testMaterial );
}


//-----------------------------------------------------------------------------------------------
void TileMap::Load()
{
	float volume = g_game->m_rng->RollRandomFloatInRange( .5f, 1.f );
	float balance = g_game->m_rng->RollRandomFloatInRange( -1.f, 1.f );
	float speed = g_game->m_rng->RollRandomFloatInRange( .5f, 2.f );

	g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( "Data/Audio/Teleporter.wav" ), false, volume, balance, speed );

	g_game->SetCameraPositionAndYaw( m_mapDef->GetPlayerStartPos(), m_mapDef->GetPlayerStartYaw() );
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
void TileMap::UpdateMeshes()
{
	m_mesh.clear();

	for ( int tileIdx = 0; tileIdx < (int)m_tiles.size(); ++tileIdx )
	{
		Tile& tile = m_tiles[tileIdx];

		if ( tile.m_regionTypeDef == nullptr )
		{
			continue;
		}

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
			// Bottom face
			Vec2 uvsAtMins, uvsAtMaxs;
			MapMaterialTypeDefinition* materialTypeDef = tile.m_regionTypeDef->GetFloorMaterial();
			materialTypeDef->GetSpriteSheet()->GetSpriteUVs( uvsAtMins, uvsAtMaxs, materialTypeDef->GetSpriteCoords() );
			if ( materialTypeDef == nullptr )
			{
				return;
			}

			AddTileFace( vert0, vert1, vert2, vert3, uvsAtMins, uvsAtMaxs );

			// Top face
			materialTypeDef = tile.m_regionTypeDef->GetCeilingMaterial();
			materialTypeDef->GetSpriteSheet()->GetSpriteUVs( uvsAtMins, uvsAtMaxs, materialTypeDef->GetSpriteCoords() );
			
			if ( materialTypeDef == nullptr )
			{
				return;
			}

			AddTileFace( vert5, vert4, vert7, vert6, uvsAtMins, uvsAtMaxs );
		}
		else
		{
			Vec2 uvsAtMins, uvsAtMaxs;
			MapMaterialTypeDefinition* materialTypeDef = tile.m_regionTypeDef->GetSideMaterial();
			if ( materialTypeDef == nullptr )
			{
				return;
			}

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
	Map::Render();

	if ( m_mesh.size() == 0 )
	{
		return;
	}

	g_renderer->SetModelMatrix( Mat44::IDENTITY );
	g_renderer->SetBlendMode( eBlendMode::ALPHA );
	g_renderer->BindShaderProgram( g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/Default.hlsl" ) );

	g_renderer->BindTexture( 0, g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" ) );

	g_renderer->DrawVertexArray( m_mesh );
}


//-----------------------------------------------------------------------------------------------
void TileMap::DebugRender() const
{
	Map::DebugRender();

	Transform cameraTransform = g_game->GetWorldCamera()->GetTransform();
	RaycastResult result = Raycast( cameraTransform.GetPosition(), cameraTransform.GetForwardVector(), 3.f );

	//DebugAddWorldLine( cameraTransform.GetPosition(), cameraTransform.GetPosition() + cameraTransform.GetForwardVector() * 3.f, Rgba8::MAGENTA, .001f );
}


//-----------------------------------------------------------------------------------------------
RaycastResult TileMap::Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	std::string message = "Hit nothing";

	RaycastResult floorResult = RaycastAgainstZPlane( startPos, forwardNormal, maxDist, 0.f );
	RaycastResult ceilingResult = RaycastAgainstZPlane( startPos, forwardNormal, maxDist, TILE_SIZE );

	if ( floorResult.didImpact )
	{
		message = "Hit floor";
		DebugAddWorldPoint( floorResult.impactPos, Rgba8::GREEN );
		DebugAddWorldArrow( floorResult.impactPos, floorResult.impactPos + floorResult.impactSurfaceNormal * .5f, Rgba8::BLUE );
	}
	else if ( ceilingResult.didImpact )
	{
		message = "Hit ceiling";
		DebugAddWorldPoint( ceilingResult.impactPos, Rgba8::GREEN );
		DebugAddWorldArrow( ceilingResult.impactPos, ceilingResult.impactPos + ceilingResult.impactSurfaceNormal * .5f, Rgba8::BLUE );
	}

	DebugAddScreenText( Vec4( 0.f, .81f, 0.f, 0.f ), Vec2::ZERO, 20.f, Rgba8::GREEN, Rgba8::GREEN, 0.f, message.c_str() );

	return RaycastResult();
}


//-----------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstZPlane( const Vec3& startPos, const Vec3& forwardNormal, float maxDist, float height ) const
{	
	Vec3 endPos = startPos + ( maxDist * forwardNormal );

	float impactFraction = RangeMapFloat( startPos.z, endPos.z, 0.f, 1.f, height );

	RaycastResult result;
	result.startPos = startPos;
	result.forwardNormal = forwardNormal;
	result.maxDist = maxDist;
	result.impactFraction = impactFraction;

	if ( impactFraction > 0.f && impactFraction <= 1.f )
	{
		result.didImpact = true;

		Vec3 castVec = endPos - startPos;
		float castDist = castVec.GetLength();
		result.impactDist = castDist * impactFraction;
		result.impactPos = startPos + forwardNormal * result.impactDist;

		result.impactSurfaceNormal = startPos.z > height ? Vec3( 0.f, 0.f, 1.f ) : Vec3( 0.f, 0.f, -1.f );
	}

	return result;
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

	if ( adjacentTile == nullptr 
		 || adjacentTile->m_regionTypeDef == nullptr )
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

	Tile* entityTile = GetTileFromWorldCoords( entity.GetPosition() );
	if ( entityTile == nullptr )
	{
		return;
	}

	std::vector<Tile*> surroundingTiles = GetTilesInRadius( *entityTile, 1, true );
	for ( int tileIdx = 0; tileIdx < (int)surroundingTiles.size(); ++tileIdx )
	{
		Tile*& tile = surroundingTiles[tileIdx];
		if ( tile != nullptr
			 && tile->IsSolid() )
		{
			PushDiscOutOfAABB2D( entity.m_position, entity.GetPhysicsRadius(), tile->GetBounds() );
		}
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

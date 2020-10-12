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
#include "Game/MapRegionTypeDefinition.hpp"
#include "Game/MapMaterialTypeDefinition.hpp"


//-----------------------------------------------------------------------------------------------
TileMap::TileMap( const MapData& mapData )
	: Map( mapData )
{
	m_dimensions = mapData.dimensions;

	BuildCardinalDirectionsArray();
	PopulateTiles( mapData.regionTypeDefs );
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
	/*float volume = g_game->m_rng->RollRandomFloatInRange( .5f, 1.f );
	float balance = g_game->m_rng->RollRandomFloatInRange( -1.f, 1.f );
	float speed = g_game->m_rng->RollRandomFloatInRange( .5f, 2.f );*/

	//g_audioSystem->PlaySound( g_audioSystem->CreateOrGetSound( "Data/Audio/Teleporter.wav" ), false, volume, balance, speed );

	g_game->SetCameraPositionAndYaw( m_playerStartPos, m_playerStartYaw );
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

	if ( g_game->g_raytraceFollowCamera )
	{
		m_raytraceTransform = g_game->GetWorldCamera()->GetTransform();
	}
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

	RaycastResult result = Raycast( m_raytraceTransform.GetPosition(), m_raytraceTransform.GetForwardVector(), 5.f );
	if ( result.didImpact )
	{
		DebugAddWorldPoint( result.impactPos, Rgba8::PURPLE );
		DebugAddWorldArrow( result.impactPos, result.impactPos + result.impactSurfaceNormal * .5f, Rgba8::ORANGE );

		if ( !g_game->g_raytraceFollowCamera )
		{
			DebugAddWorldArrow( m_raytraceTransform.GetPosition(), result.impactPos, Rgba8::MAGENTA );
		}
	}
}


//-----------------------------------------------------------------------------------------------
RaycastResult TileMap::Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	RaycastResult closestImpact;
	closestImpact.impactDist = maxDist;

	RaycastResult floorResult = RaycastAgainstZPlane( startPos, forwardNormal, maxDist, 0.f );
	if ( floorResult.didImpact
		 && floorResult.impactDist < closestImpact.impactDist )
	{
		closestImpact = floorResult;
	}

	RaycastResult ceilingResult = RaycastAgainstZPlane( startPos, forwardNormal, maxDist, TILE_SIZE );
	if ( ceilingResult.didImpact
		 && ceilingResult.impactDist < closestImpact.impactDist )
	{
		closestImpact = ceilingResult;
	}

	RaycastResult wallsResult = RaycastAgainstWalls( startPos, forwardNormal, maxDist );
	if ( wallsResult.didImpact
		 && wallsResult.impactDist < closestImpact.impactDist )
	{
		closestImpact = wallsResult;
	}

	RaycastResult entitiesResult = RaycastAgainstEntitiesFast( startPos, forwardNormal, maxDist );
	if ( entitiesResult.didImpact
		 && entitiesResult.impactDist < closestImpact.impactDist )
	{
		closestImpact = entitiesResult;
	}

	return closestImpact;
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
RaycastResult TileMap::RaycastAgainstWalls( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	RaycastResult result;
	result.startPos = startPos;
	result.forwardNormal = forwardNormal;
	result.maxDist = maxDist;

	// Check if starting tile is solid
	const Tile* startTile = GetTileFromWorldCoords( startPos.XY() );
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
	Vec2 rayDisp = forwardNormal.XY() * maxDist;

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
				Vec3 startToImpact( forwardNormal * dOfNextXCrossing );
				result.impactPos = startPos + startToImpact;
				result.impactDist = dOfNextXCrossing;
				result.impactSurfaceNormal = Vec3( (float)-tileStepDirX, 0.f, 0.f );
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
				Vec3 startToImpact( forwardNormal * dOfNextYCrossing );
				result.impactPos = startPos + startToImpact;
				result.impactDist = dOfNextYCrossing;
				result.impactSurfaceNormal = Vec3( 0.f, (float)-tileStepDirY, 0.f );
				break;
			}

			dOfNextYCrossing += yDeltaDistAlongRay;
		}
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstEntities( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	RaycastResult result;
	result.startPos = startPos;
	result.forwardNormal = forwardNormal;
	result.maxDist = maxDist;

	// Step and sample to detect entity ray collision
	constexpr int maxStepCount = 1000;
	float stepIncrement = maxDist / (float)maxStepCount;
	float curStepIncrement = 0.f;
	for ( int stepCount = 0; stepCount < maxStepCount; ++stepCount )
	{
		for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
		{
			Entity* const& entity = m_entities[entityIdx];
			if ( entity == nullptr )
			{
				continue;
			}

			Vec3 samplePos = startPos + forwardNormal * curStepIncrement;
			// Make sure entity is within z range
			// TODO: Update to account for flying enemies
			if ( samplePos.z <= 0.f
				 || samplePos.z > entity->GetHeight() )
			{
				continue;
			}

			// Check if sample is inside entity
			if ( !IsPointInsideDisc( samplePos.XY(), entity->GetPosition(), entity->GetPhysicsRadius() ) )
			{
				continue;
			}

			result.didImpact = true;
			result.impactFraction = curStepIncrement / maxDist;
			result.impactPos = samplePos;
			result.impactDist = ( samplePos - startPos ).GetLength();
			result.impactEntity = entity;

			// Detect if collision is with top of cylinder
			if ( IsNearlyEqual( samplePos.z, entity->GetHeight(), .001f ) 
				 && startPos.z > entity->GetHeight() )
			{
				result.impactSurfaceNormal = Vec3( 0.f, 0.f, 1.f );
			}
			// TODO: Update for flying enemies
			else if ( IsNearlyEqual( samplePos.z, 0.f, .001f )
					  && startPos.z < 0.f )
			{
				result.impactSurfaceNormal = Vec3( 0.f, 0.f, -1.f );
			}
			else
			{
				result.impactSurfaceNormal = ( samplePos - Vec3( entity->GetPosition(), samplePos.z ) ).GetNormalized();
			}

			return result;
		}

		curStepIncrement += stepIncrement;
	}
		
	return result;
}


//-----------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstEntitiesFast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	RaycastResult result;
	result.startPos = startPos;
	result.forwardNormal = forwardNormal;
	result.maxDist = maxDist;
	result.impactDist = maxDist;

	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr 
			 || entity->IsPossessed() )
		{
			continue;
		}

		// i and j in terms of the forward vector's space
		Vec2 iBasis = forwardNormal.XY().GetNormalized();
		Vec2 jBasis = iBasis.GetRotated90Degrees();

		// Project disc into forward vector's space
		Vec2 displacementFromStartToCenterOfDisc = entity->m_position - startPos.XY();
		Vec2 posOfCircleCenterAlongRay( DotProduct2D( iBasis, displacementFromStartToCenterOfDisc ), DotProduct2D( jBasis, displacementFromStartToCenterOfDisc ) );

		// TODO: Account for y out of reasonable area also
		if ( maxDist < posOfCircleCenterAlongRay.x - entity->GetPhysicsRadius()
			 || posOfCircleCenterAlongRay.x + entity->GetPhysicsRadius() < 0.f )
		{
			continue;
		}

		// Use pythagorean theorum where 
		// b = side of triangle made by disc center to closest point on line
		// c = hypoteneuse of triangle made by disc radius
		// a = side from nearest point on line to disc center to intersection point
		float bSquared = posOfCircleCenterAlongRay.y * posOfCircleCenterAlongRay.y;
		float cSquared = entity->GetPhysicsRadius() * entity->GetPhysicsRadius();
		float aSquared = cSquared - bSquared;

		FloatRange dOverlap;
		// Did not overlap
		if ( aSquared < 0.f )
		{
			continue;
		}
		// Touching the edge, only 1 intersection point
		else if ( IsNearlyEqual( aSquared, 0.f ) )
		{
			dOverlap.min = posOfCircleCenterAlongRay.x;
			dOverlap.max = posOfCircleCenterAlongRay.x;
		}
		// Forard vector has 2 intersection points with disc
		else
		{
			float a = sqrtf( aSquared );
			dOverlap.min = posOfCircleCenterAlongRay.x - a;
			dOverlap.max = posOfCircleCenterAlongRay.x + a;
		}
		
		// If the min intersection point is behind the start of the forward vector, clamp it to vector start
		dOverlap.min = ClampMin( dOverlap.min, 0.f );

		// Project XY along the XYZ forward to see how far along it went
		float maxDistXY = DotProduct3D( Vec3( forwardNormal.XY(), 0.f ).GetNormalized(), forwardNormal ) * maxDist;
		float tOverlapMin = dOverlap.min / maxDistXY;
				
		float impactDist = tOverlapMin * maxDist;
		Vec3 impactDisp = forwardNormal * impactDist;
		Vec3 impactPos = startPos + impactDisp;

		if ( impactDist < result.impactDist )
		{
			if ( !DoesRayHitEntityAlongZ( result, impactPos, *entity ) )
			{
				continue;
			}

			result.didImpact = true;
			result.impactFraction = dOverlap.min / maxDist;
			result.impactDist = impactDist;
			result.impactEntity = entity;
		}
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
bool TileMap::DoesRayHitEntityAlongZ( RaycastResult& raycastResult, const Vec3& potentialImpactPos, const Entity& entity ) const
{
	Vec3 impactPos = potentialImpactPos;

	// 3 cases
	// Hit side first
	if ( impactPos.z > 0.f
		 && impactPos.z < entity.GetHeight() )
	{
		raycastResult.impactPos = impactPos;
		raycastResult.impactSurfaceNormal = ( impactPos - Vec3( entity.GetPosition(), impactPos.z ) ).GetNormalized();

		return true;
	}
	// Hit top first
	else if ( raycastResult.startPos.z > entity.GetHeight() )
	{
		RaycastResult rayAgainstTop = RaycastAgainstZPlane( raycastResult.startPos, raycastResult.forwardNormal, raycastResult.maxDist, entity.GetHeight() );

		if ( rayAgainstTop.didImpact )
		{
			if ( IsPointInsideDiscFast( rayAgainstTop.impactPos.XY(), entity.GetPosition(), entity.GetPhysicsRadius() ) )
			{
				raycastResult.impactPos = rayAgainstTop.impactPos;
				raycastResult.impactSurfaceNormal = Vec3( 0.f, 0.f, 1.f );

				return true;
			}
		}
	}
	// Hit bottom first
	else if ( raycastResult.startPos.z < 0.f )
	{
		RaycastResult rayAgainstBottom = RaycastAgainstZPlane( raycastResult.startPos, raycastResult.forwardNormal, raycastResult.maxDist, 0.f );
		if ( rayAgainstBottom.didImpact )
		{
			if ( IsPointInsideDiscFast( rayAgainstBottom.impactPos.XY(), entity.GetPosition(), entity.GetPhysicsRadius() ) )
			{
				raycastResult.impactPos = rayAgainstBottom.impactPos;
				raycastResult.impactSurfaceNormal = Vec3( 0.f, 0.f, -1.f );
				
				return true;
			}
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void TileMap::PopulateTiles( const std::vector<MapRegionTypeDefinition*>& regionTypeDefs )
{
	CreateInitialTiles( regionTypeDefs );
	//SolidifySurroundingTiles();
}


//-----------------------------------------------------------------------------------------------
void TileMap::CreateInitialTiles( const std::vector<MapRegionTypeDefinition*>& regionTypeDefs )
{
	for ( int y = 0; y < m_dimensions.y; ++y )
	{
		for ( int x = 0; x < m_dimensions.x; ++x )
		{
			m_tiles.push_back( Tile( IntVec2( x, y ), regionTypeDefs[( y * m_dimensions.x ) + x] ) );
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
		 || adjacentTile->m_regionTypeDef == nullptr )
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
		 || tile->m_regionTypeDef == nullptr )
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

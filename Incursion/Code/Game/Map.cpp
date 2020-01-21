#include "Game/Map.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/RaycastImpact.hpp"
#include "Game/MapDefinition.hpp"

#include "Game/Player.hpp"
#include "Game/NpcTurret.hpp"
#include "Game/NpcTank.hpp"
#include "Game/Boulder.hpp"
#include "Game/Bullet.hpp"
#include "Game/Explosion.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( int id, World* world, const MapDefinition& mapDefinition )
	: m_id( id )
	, m_world( world )
	, m_width( mapDefinition.m_dimensions.x )
	, m_height( mapDefinition.m_dimensions.y )
{
	PopulateTiles( mapDefinition );
	PopulateEnemySpawnLocations( mapDefinition.m_mapEntityDefinition );

	m_tileTexture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	// Start after the player list, let world delete player
	for ( int entityTypeIndex = 1; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityTypeIndex].size(); ++entityIndex )
		{
			Entity*& entity = m_entityVectorsByType[entityTypeIndex][entityIndex];
			if ( entity != nullptr )
			{
				delete entity;
				entity = nullptr;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateEntities( deltaSeconds );
	if ( CheckForWin() )
	{
		return;
	}

	ResolveCollisions();
	UpdateCameras();

	DeleteGarbageEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateEntities( float deltaSeconds )
{
	for ( int entityType = 0; entityType < NUM_ENTITY_TYPES; ++entityType )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityType].size(); ++entityIndex )
		{
			Entity* entity = m_entityVectorsByType[entityType][entityIndex];
			if ( entity != nullptr )
			{
				entity->Update( deltaSeconds );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool Map::CheckForWin()
{
	if ( m_player == nullptr 
		 || m_player->IsDead() )
	{
		return false;
	}

	if ( IsPointInsideDisc( m_exitPosition, m_player->m_position, m_player->m_physicsRadius ) )
	{
		m_player->m_position = m_player->m_startingPosition;
		m_world->ChangeMap( m_id + 1 );
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateCameras()
{
	if ( g_game->IsDebugCameraEnabled() )
	{
		Vec2 aspectDimensions = Vec2( WINDOW_WIDTH, WINDOW_HEIGHT );
		AABB2 cameraBounds( Vec2( 0.f, 0.f ), aspectDimensions );
		cameraBounds.StretchToIncludePointMaintainAspect( Vec2( (float)m_width, (float)m_height ), aspectDimensions );
		
		g_game->SetWorldCameraOrthographicView( cameraBounds );
	}
	else
	{
		CenterCameraOnPlayer();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::CenterCameraOnPlayer() const
{
	if ( m_player != nullptr )
	{
		Vec2 halfWindowSize( WINDOW_WIDTH * .5f, WINDOW_HEIGHT * .5f );
		AABB2 cameraBounds( m_player->GetPosition() - halfWindowSize, m_player->GetPosition() + halfWindowSize );

		AABB2 windowBox( Vec2( 0.f, 0.f ), Vec2( (float)m_width, (float)m_height ) );
		cameraBounds.FitWithinBounds( windowBox );

		g_game->SetWorldCameraOrthographicView( cameraBounds );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::Render() const
{
	RenderTiles();
	RenderEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::RenderHUD() const
{
	m_player->RenderHUD();
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
	DebugRenderEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::LoadMap()
{
	SpawnEnemies();
}


//-----------------------------------------------------------------------------------------------
void Map::SpawnEnemies()
{
	for ( int tankIndex = 0; tankIndex < (int)m_entitySpawnPositionVectorsByType[ENTITY_TYPE_NPC_TANK].size(); ++tankIndex )
	{
		SpawnNewEntity( ENTITY_TYPE_NPC_TANK, ENTITY_FACTION_EVIL, m_entitySpawnPositionVectorsByType[ENTITY_TYPE_NPC_TANK][tankIndex] );
	}

	for ( int turretIndex = 0; turretIndex < (int)m_entitySpawnPositionVectorsByType[ENTITY_TYPE_NPC_TURRET].size(); ++turretIndex )
	{
		SpawnNewEntity( ENTITY_TYPE_NPC_TURRET, ENTITY_FACTION_EVIL, m_entitySpawnPositionVectorsByType[ENTITY_TYPE_NPC_TURRET][turretIndex] );
	}

	for ( int boulderIndex = 0; boulderIndex < (int)m_entitySpawnPositionVectorsByType[ENTITY_TYPE_BOULDER].size(); ++boulderIndex )
	{
		SpawnNewEntity( ENTITY_TYPE_BOULDER, ENTITY_FACTION_NEUTRAL, m_entitySpawnPositionVectorsByType[ENTITY_TYPE_BOULDER][boulderIndex] );
	}

	ResolveTileCollisions();
}


//-----------------------------------------------------------------------------------------------
void Map::SpawnExplosion( const Vec2& position, float radius, float durationSeconds )
{
	Explosion* explosion = new Explosion( this, position, radius, durationSeconds );
	AddEntityToVector( explosion, ENTITY_TYPE_EXPLOSION );
}


//-----------------------------------------------------------------------------------------------
void Map::UnloadMap()
{
	// Player will be handled separately
	for ( int entityType = 1; entityType < NUM_ENTITY_TYPES; ++entityType )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityType].size(); ++entityIndex )
		{
			Entity*& entity = m_entityVectorsByType[entityType][entityIndex];
			if ( entity != nullptr )
			{
				delete entity;
				entity = nullptr;
			}
		}

		m_entityVectorsByType[entityType].clear();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::RenderTiles() const
{
	std::vector<Vertex_PCU> vertexes;

	for ( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		const Tile& tile = m_tiles[tileIndex];
		AABB2 uvCoords = GetUVsForTileType( tile.m_tileType );
		g_renderer->AppendVertsForAABB2D( vertexes, tile.GetBounds(), GetColorForTileType( tile.m_tileType ), uvCoords.mins, uvCoords.maxs );
	}

	g_renderer->BindTexture( m_tileTexture );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Map::RenderEntities() const
{
	for ( int entityType = 0; entityType < NUM_ENTITY_TYPES; ++entityType )
	{
		if ( entityType == ENTITY_TYPE_EXPLOSION )
		{
			g_renderer->SetBlendMode( BlendMode::ADDITIVE );
		}

		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityType].size(); ++entityIndex )
		{
			Entity*const& entity = m_entityVectorsByType[entityType][entityIndex];
			if ( entity != nullptr )
			{
				entity->Render();
			}
		}

		if ( entityType == ENTITY_TYPE_EXPLOSION )
		{
			g_renderer->SetBlendMode( BlendMode::ALPHA );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRenderEntities() const
{
	for ( int entityType = 0; entityType < NUM_ENTITY_TYPES; ++entityType )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityType].size(); ++entityIndex )
		{
			Entity*const& entity = m_entityVectorsByType[entityType][entityIndex];
			if ( entity != nullptr )
			{
				entity->DebugRender();
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveCollisions()
{
	ResolveBulletEnvironmentCollisions();
	ResolveEntityCollisions();
	ResolveTileCollisions();
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityCollisions()
{
	// Loop through each entity vector to check collisions
	for ( int entityType1 = 0; entityType1 < NUM_ENTITY_TYPES; ++entityType1 )
	{
		for ( int entityIndex1 = 0; entityIndex1 < (int)m_entityVectorsByType[entityType1].size(); ++entityIndex1 )
		{
			Entity*& entity1 = m_entityVectorsByType[entityType1][entityIndex1];
			if ( entity1 == nullptr 
				 || entity1->IsDead() )
			{
				continue;
			}

			// Loop through each entity again and resolve each collision
			for ( int entityType2 = 0; entityType2 < NUM_ENTITY_TYPES; ++entityType2 )
			{
				for ( int entityIndex2 = 0; entityIndex2 < (int)m_entityVectorsByType[entityType2].size(); ++entityIndex2 )
				{
					Entity*& entity2 = m_entityVectorsByType[entityType2][entityIndex2];
					if ( entity2 == nullptr 
						 || entity2->IsDead() 
						 || entity1 == entity2 )
					{
						continue;
					}
					
					// Check bullet damage before trying to resolve entity collision 
					if ( entityType1 == ENTITY_TYPE_GOOD_BULLET
						 || entityType1 == ENTITY_TYPE_EVIL_BULLET )
					{
						if ( entity2->IsHitByBullets()
							 && entity1->GetFaction() != entity2->GetFaction()
							 && DoEntitiesOverlap( *entity1, *entity2 ))
						{
							if ( entity2->GetType() == ENTITY_TYPE_BOULDER )
							{
								ReflectBulletOffDisc( *entity1, entity2->GetPosition(), entity2->GetPhysicsRadius() );
							}
							else
							{
								entity2->TakeDamage( 1 );
								entity1->Die();
								break;
							}
						}
					}

					ResolveEntityCollision( *entity1, *entity2 );
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveTileCollisions()
{
	for ( int entityType = 0; entityType < NUM_ENTITY_TYPES; ++entityType )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityType].size(); ++entityIndex )
		{
			Entity* const& entity = m_entityVectorsByType[entityType][entityIndex];
			if ( entity == nullptr )
			{
				continue;
			}

			// Check if tile collisions are enabled for player
			if ( entityType == ENTITY_TYPE_PLAYER && g_game->IsNoClipEnabled() )
			{
				continue;
			}
			
			ResolveEntityCollisionsWithSurroundingTiles( *entity );
			ApplyTileEffectsToEntity( *entity );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityCollision( Entity& a, Entity& b )
{
	if ( !a.IsPushedByEntities() && !b.IsPushedByEntities() )
	{
		return;
	}

	if ( !a.PushesEntities() && !b.PushesEntities() )
	{
		return;
	}

	if ( !DoEntitiesOverlap( a, b ) )
	{
		return;
	}

	if( a.PushesEntities() && b.IsPushedByEntities() && !a.IsPushedByEntities() )
	{
		PushEntityOutOfEntity( b, a );
	}
	else if( b.PushesEntities() && a.IsPushedByEntities() && !b.IsPushedByEntities() )
	{
		PushEntityOutOfEntity( a, b );
	}
	else if( a.PushesEntities() && b.PushesEntities() && a.IsPushedByEntities() && b.IsPushedByEntities() )
	{
		PushEntitiesOutOfEachOther( a, b );
	}
	else if ( a.PushesEntities() && !b.PushesEntities() && a.IsPushedByEntities() && b.IsPushedByEntities() )
	{
		PushEntitiesOutOfEachOther( a, b );
	}
	else if ( !a.PushesEntities() && b.PushesEntities() && a.IsPushedByEntities() && b.IsPushedByEntities() )
	{
		PushEntitiesOutOfEachOther( a, b );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityCollisionsWithSurroundingTiles( Entity& entity )
{
	// Check if entity ignores collision with walls
	if ( !entity.IsPushedByWalls() )
	{
		return;
	}

	// Check tiles in following order for collision
	// 5 1 6
	// 4 * 2
	// 8 3 7
	for ( int posIndex = 0; posIndex < (int)m_surroundingTilePositions.size(); ++posIndex )
	{
		ResolveEntityCollisionWithTile( entity, entity.m_position + m_surroundingTilePositions[posIndex] );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityCollisionWithTile( Entity& entity, Vec2 tilePosition )
{
	int entityTileIndex = GetTileIndexFromTileCoords( (int)tilePosition.x, (int)tilePosition.y );
	if ( entityTileIndex < 0
		|| entityTileIndex >= (int)m_tiles.size() )
	{
		return;
	}

	const Tile& entityTile = m_tiles[entityTileIndex];
	if ( IsTileTypeSolid( entityTile.m_tileType ) )
	{
		Vec2 newPosition( entity.m_position );
		PushDiscOutOfAABB2D( newPosition, entity.GetPhysicsRadius(), entityTile.GetBounds() );
		entity.m_position = newPosition;
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ApplyTileEffectsToEntity( Entity& entity )
{
	// Include current tile in check
	std::vector<Vec2> tilePositionsToCheck( m_surroundingTilePositions );
	tilePositionsToCheck.push_back( Vec2::ZERO );

	// TODO: When more effect tiles are added, use a better strategy to layer effects
	for ( int posIndex = 0; posIndex < (int)tilePositionsToCheck.size(); ++posIndex )
	{
		Tile* tile = GetTileFromWorldCoords( entity.m_position + tilePositionsToCheck[posIndex] );
		if ( tile == nullptr
			|| !DoDiscAndAABBOverlap2D( entity.m_position, entity.m_physicsRadius, tile->GetBounds() ) )
		{
			continue;
		}

		if ( tile != nullptr
			 && ( tile->m_tileType == TILE_TYPE_MUD || tile->m_tileType == TILE_TYPE_WET_SAND ) )
		{
			entity.m_movementSpeedFraction = .5f;
			return;
		}
	}

	// Not touching any special tiles
	entity.m_movementSpeedFraction = 1.f;
}


//-----------------------------------------------------------------------------------------------
void Map::ReflectBulletOffDisc( Entity& bullet, const Vec2& center, float radius )
{	
	Vec2 normalStart = GetNearestPointOnDisc2D( bullet.GetPosition(), center, radius );
	Vec2 normal = GetNormalizedDirectionFromAToB( center, normalStart );

	PushDiscOutOfDisc2D( bullet.m_position, bullet.GetPhysicsRadius(), center, radius );

	bullet.m_velocity = bullet.m_velocity.GetReflected( normal );
	bullet.m_orientationDegrees = bullet.m_velocity.GetOrientationDegrees();
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveBulletEnvironmentCollisions()
{
	// Check evil bullets
	for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[ENTITY_TYPE_EVIL_BULLET].size(); ++entityIndex )
	{
		Entity* evilBullet = m_entityVectorsByType[ENTITY_TYPE_EVIL_BULLET][entityIndex];
		if ( evilBullet == nullptr )
		{
			continue;
		}

		if ( IsBulletInsideWall( *evilBullet ) )
		{
			evilBullet->Die();
			continue;
		}

		if ( IsBulletOutsideLevel( *evilBullet ) )
		{
			evilBullet->Die();
			continue;
		}
	}

	// Check good bullets
	for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[ENTITY_TYPE_GOOD_BULLET].size(); ++entityIndex )
	{
		Entity* goodBullet = m_entityVectorsByType[ENTITY_TYPE_GOOD_BULLET][entityIndex];
		if ( goodBullet == nullptr )
		{
			continue;
		}

		if ( IsBulletInsideWall( *goodBullet ) )
		{
			goodBullet->Die();
			continue;
		}

		if ( IsBulletOutsideLevel( *goodBullet ) )
		{
			goodBullet->Die();
			continue;
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool Map::IsBulletInsideWall( const Entity& bullet )
{
	// Check tiles in following order for collision
	// 5 1 6
	// 4 * 2
	// 8 3 7
	//TODO: Add zero into surrounding vector and skip in entity collision
	std::vector<Vec2> surroundingTilePositions( m_surroundingTilePositions );
	surroundingTilePositions.push_back( Vec2::ZERO );
	for ( int posIndex = 0; posIndex < (int)surroundingTilePositions.size(); ++posIndex )
	{
		Vec2 tilePosition( bullet.m_position + surroundingTilePositions[posIndex] );
		int entityTileIndex = GetTileIndexFromTileCoords( (int)tilePosition.x, (int)tilePosition.y );
		if ( entityTileIndex < 0
			 || entityTileIndex >= (int)m_tiles.size() )
		{
			continue;
		}

		const Tile& entityTile = m_tiles[entityTileIndex];
		if( DoesTileTypeStopBullets( entityTile.m_tileType )
			&& DoDiscAndAABBOverlap2D( bullet.GetPosition(), bullet.GetPhysicsRadius(), entityTile.GetBounds() ) )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Map::IsBulletOutsideLevel( const Entity& bullet )
{
	AABB2 levelBounds( Vec2::ZERO, Vec2( (float)m_width, (float)m_height ) );

	return !DoDiscAndAABBOverlap2D( bullet.m_position, bullet.m_cosmeticRadius, levelBounds );
}


//-----------------------------------------------------------------------------------------------
bool Map::DoEntitiesOverlap( const Entity& entity1, const Entity& entity2 )
{
	return DoDiscsOverlap( entity1.GetPosition(), entity1.GetPhysicsRadius(), entity2.GetPosition(), entity2.GetPhysicsRadius() );
}


//-----------------------------------------------------------------------------------------------
void Map::PushEntityOutOfEntity( Entity& entityToPush, const Entity& entityPushing )
{
	PushDiscOutOfDisc2D( entityToPush.m_position, entityToPush.GetPhysicsRadius(), entityPushing.GetPosition(), entityPushing.GetPhysicsRadius() );
}


//-----------------------------------------------------------------------------------------------
void Map::PushEntitiesOutOfEachOther( Entity& entity1, Entity& entity2 )
{
	PushDiscsOutOfEachOther2D( entity1.m_position, entity1.GetPhysicsRadius(), entity2.m_position, entity2.GetPhysicsRadius() );
}


//-----------------------------------------------------------------------------------------------
bool Map::IsPointInSolid( const Vec2& point )
{
	Tile* tile = GetTileFromWorldCoords( point );
	if ( tile == nullptr )
	{
		return true;
	}

	return IsTileSolid( *tile );
}


//-----------------------------------------------------------------------------------------------
bool Map::IsPointInVisionBlockingSolid( const Vec2& point )
{
	Tile* tile = GetTileFromWorldCoords( point );
	if ( tile == nullptr )
	{
		return true;
	}

	return DoesTileBlockVision( *tile );
}


//-----------------------------------------------------------------------------------------------
bool Map::IsTileSolid( const Tile& tile )
{
	return IsTileTypeSolid( tile.m_tileType );
}


//-----------------------------------------------------------------------------------------------
bool Map::DoesTileBlockVision( const Tile& tile )
{
	return IsTileTypeSolid( tile.m_tileType ) && DoesTileTypeStopBullets( tile.m_tileType );
}


//-----------------------------------------------------------------------------------------------
RaycastImpact Map::Raycast( const Vec2& start, const Vec2& forwardDirection, float maxDistance, float stepSize )
{
	for ( float raycastStep = 0; raycastStep < maxDistance; raycastStep += stepSize )
	{
		Vec2 raycastPos = start + ( forwardDirection * raycastStep );

		if ( IsPointInSolid( raycastPos ) )
		{
			return RaycastImpact( true, raycastPos );
		}
	}

	return RaycastImpact( false, start + ( forwardDirection * maxDistance ) );
}


//-----------------------------------------------------------------------------------------------
RaycastImpact Map::VisionRaycast( const Vec2& start, const Vec2& forwardDirection, float maxDistance, float stepSize /*= 0.01f */ )
{
	for ( float raycastStep = 0; raycastStep < maxDistance; raycastStep += stepSize )
	{
		Vec2 raycastPos = start + ( forwardDirection * raycastStep );

		if ( IsPointInVisionBlockingSolid( raycastPos ) )
		{
			return RaycastImpact( true, raycastPos );
		}
	}

	return RaycastImpact( false, start + ( forwardDirection * maxDistance ) );
}


//-----------------------------------------------------------------------------------------------
bool Map::HasLineOfSight( const Entity& source, const Entity& target )
{
	Vec2 sourcePos = source.GetPosition();
	Vec2 targetPos = target.GetPosition();

	RaycastImpact impact = VisionRaycast( sourcePos, GetNormalizedDirectionFromAToB( sourcePos, targetPos ), GetDistance2D( sourcePos, targetPos ) );
	
	return !impact.m_didImpact;
}


//-----------------------------------------------------------------------------------------------
void Map::PopulateTiles( const MapDefinition& mapDefinition )
{
	FloodFillResult result;
	while ( !result.m_isExitReachable )
	{
		m_tiles.clear();
		CreateInitialTiles( mapDefinition.m_defaultTileType );
		DrawBorder( mapDefinition.m_edgeTileType );
		DeployWorms( mapDefinition.m_tileWorms );
		ClearStartSafeZone( mapDefinition.m_startTileType, mapDefinition.m_shieldTileType );
		ClearEndSafeZone( mapDefinition.m_exitTileType, mapDefinition.m_shieldTileType );
		AddExitPoint();
		BuildSurroundingTileVector();
		result = FloodFillMap( Vec2( PLAYER_START_X, PLAYER_START_Y ) );
	} 

	m_floodFillResult = result;
	FillInUnreachableTiles( mapDefinition.m_edgeTileType );
}


//-----------------------------------------------------------------------------------------------
void Map::CreateInitialTiles( TileType defaultTile )
{
	for ( int y = 0; y < m_height; ++y )
	{
		for ( int x = 0; x < m_width; ++x )
		{
			m_tiles.push_back( Tile( x, y, defaultTile ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DrawBorder( TileType edgeTileType )
{
	for ( int xIndex = 0; xIndex < m_width; ++xIndex )
	{
		m_tiles[GetTileIndexFromTileCoords( xIndex, 0 )].m_tileType = edgeTileType;
		m_tiles[GetTileIndexFromTileCoords( xIndex, m_height - 1 )].m_tileType = edgeTileType;
	}

	for ( int yIndex = 0; yIndex < m_height; ++yIndex )
	{
		m_tiles[GetTileIndexFromTileCoords( 0, yIndex )].m_tileType = edgeTileType;
		m_tiles[GetTileIndexFromTileCoords( m_width - 1, yIndex )].m_tileType = edgeTileType;
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DeployWorms( std::vector<TileWorm> tileWorms )
{
	for ( int wormTypeIndex = 0; wormTypeIndex < (int)tileWorms.size(); ++wormTypeIndex )
	{
		const TileWorm& worm = tileWorms[ wormTypeIndex ];
		for ( int wormIndex = 0; wormIndex < worm.m_numWorms; ++wormIndex )
		{
			IntVec2 nextPosition( g_game->m_rng->RollRandomIntInRange( 1, m_width - 2 ), g_game->m_rng->RollRandomIntInRange( 1, m_height - 2 ) );
			for ( int tileIndex = 0; tileIndex < worm.m_wormLength; ++tileIndex )
			{
				m_tiles[ GetTileIndexFromTileCoords( nextPosition ) ].m_tileType = worm.m_wormTileType;
				
				nextPosition = GetNextWormPosition( nextPosition );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
IntVec2 Map::GetNextWormPosition( const IntVec2& currentPosition )
{
	IntVec2 nextPosition( -1, -1 );
	IntVec2 cardinalDirections[4] = 
	{
		IntVec2( -1, 0 ),
		IntVec2( 1, 0 ),
		IntVec2( 0, 1 ),
		IntVec2( 0, -1 )
	};

	while ( nextPosition.x < 1 || nextPosition.x > m_width - 2
			|| nextPosition.y < 1 || nextPosition.y > m_height - 2 )
	{
		int stepDirection = g_game->m_rng->RollRandomIntInRange( 0, 3 );
		nextPosition = currentPosition + cardinalDirections[ stepDirection ];
	} 
	
	return nextPosition;
}


//-----------------------------------------------------------------------------------------------
void Map::ClearStartSafeZone( TileType startTileType, TileType shieldTileType )
{
	for ( int y = 1; y < SAFE_ZONE_SIZE + 1; ++y )
	{
		for ( int x = 1; x < SAFE_ZONE_SIZE + 1; ++x )
		{
			m_tiles[GetTileIndexFromTileCoords( x, y )].m_tileType = startTileType;
		}
	}

	// Add a protective shield around start
	m_tiles[GetTileIndexFromTileCoords( SAFE_ZONE_SIZE, SAFE_ZONE_SIZE )].m_tileType = shieldTileType;
	m_tiles[GetTileIndexFromTileCoords( SAFE_ZONE_SIZE, SAFE_ZONE_SIZE - 1 )].m_tileType = shieldTileType;
	m_tiles[GetTileIndexFromTileCoords( SAFE_ZONE_SIZE, SAFE_ZONE_SIZE - 2 )].m_tileType = shieldTileType;
	m_tiles[GetTileIndexFromTileCoords( SAFE_ZONE_SIZE - 1, SAFE_ZONE_SIZE )].m_tileType = shieldTileType;
	m_tiles[GetTileIndexFromTileCoords( SAFE_ZONE_SIZE - 2, SAFE_ZONE_SIZE )].m_tileType = shieldTileType;
}


//-----------------------------------------------------------------------------------------------
void Map::ClearEndSafeZone( TileType exitTileType, TileType shieldTileType )
{
	for ( int y = m_height - 2; y > m_height - ( SAFE_ZONE_SIZE + 2 ); --y )
	{
		for ( int x = m_width - 2; x > m_width - ( SAFE_ZONE_SIZE + 2 ); --x )
		{
			m_tiles[GetTileIndexFromTileCoords( x, y )].m_tileType = exitTileType;
		}
	}

	// Add a protective shield around exit
	int startWidth = m_width - 1 - SAFE_ZONE_SIZE;
	int startHeight = m_height - 1 - SAFE_ZONE_SIZE;
	m_tiles[GetTileIndexFromTileCoords( startWidth,		startHeight )].m_tileType = shieldTileType;
	m_tiles[GetTileIndexFromTileCoords( startWidth,		startHeight + 1 )].m_tileType = shieldTileType;
	m_tiles[GetTileIndexFromTileCoords( startWidth,		startHeight + 2 )].m_tileType = shieldTileType;
	m_tiles[GetTileIndexFromTileCoords( startWidth + 1,	startHeight )].m_tileType = shieldTileType;
	m_tiles[GetTileIndexFromTileCoords( startWidth + 2,	startHeight )].m_tileType = shieldTileType;
}


//-----------------------------------------------------------------------------------------------
void Map::AddExitPoint()
{
	float offset = ( 2.f * TILE_SIZE ) - .5f;
	m_exitPosition = Vec2( m_width - offset, m_height - offset );
	Tile* exitTile = GetTileFromWorldCoords( m_exitPosition );
	exitTile->m_tileType = TILE_TYPE_EXIT;
}


//-----------------------------------------------------------------------------------------------
void Map::BuildSurroundingTileVector()
{
	// Positions of surrounding tiles in following order
	// 5 1 6
	// 4 * 2
	// 8 3 7
	m_surroundingTilePositions.push_back( Vec2( 0.f, TILE_SIZE ) );
	m_surroundingTilePositions.push_back( Vec2( TILE_SIZE, 0.f ) );
	m_surroundingTilePositions.push_back( Vec2( 0.f, -TILE_SIZE ) );
	m_surroundingTilePositions.push_back( Vec2( -TILE_SIZE, 0.f ) );
	m_surroundingTilePositions.push_back( Vec2( -TILE_SIZE, TILE_SIZE ) );
	m_surroundingTilePositions.push_back( Vec2( TILE_SIZE, TILE_SIZE ) );
	m_surroundingTilePositions.push_back( Vec2( TILE_SIZE, -TILE_SIZE ) );
	m_surroundingTilePositions.push_back( Vec2( -TILE_SIZE, -TILE_SIZE ) );
}


//-----------------------------------------------------------------------------------------------
void Map::FillInUnreachableTiles( TileType fillType )
{
	for ( int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex )
	{
		Tile& tile = m_tiles[ tileIndex ];
		if ( !m_floodFillResult.m_tilesReachable[ tileIndex ]
			 && !IsTileSolid( tile ) )
		{
			tile.m_tileType = fillType;
		}
	}
}


//-----------------------------------------------------------------------------------------------
FloodFillResult Map::FloodFillMap( const Vec2& startPos )
{
	FloodFillResult result;
	result.m_tilesProcessed.reserve( (int)m_tiles.size() );
	result.m_tilesReachable.reserve( (int)m_tiles.size() );

	for ( int tileCount = 0; tileCount < (int)m_tiles.size(); ++tileCount )
	{
		result.m_tilesProcessed.push_back( false );
		result.m_tilesReachable.push_back( false );
	}

	int startIndex = GetTileIndexFromWorldCoords( startPos );
	// Return false for all data if start position is outside map or solid
	if ( startIndex < 0
		 || startIndex >( int )m_tiles.size() - 1
		 || IsTileSolid(m_tiles[startIndex]) )
	{
		return result;
	}

	// Seed flood fill with our starting tile
	result.m_tilesReachable[startIndex] = true;

	bool changesWereMade = true;
	while ( changesWereMade )
	{
		changesWereMade = false;

		for ( int tileIndex = 0; tileIndex < (int)m_tiles.size(); ++tileIndex )
		{
			const Tile& tile = m_tiles[ tileIndex ];
			if ( result.m_tilesProcessed[ tileIndex ]
				 || !result.m_tilesReachable[ tileIndex ] )
			{
				continue;
			}

			Vec2 worldCoords( GetWorldCoordsFromTile( tile ) );

			if ( tile.m_tileType == TILE_TYPE_EXIT )
			{
				result.m_isExitReachable = true;
			}

			Tile* northTile = GetTileFromWorldCoords( worldCoords + m_surroundingTilePositions[(int)SurroundingTiles::NORTH] );
			SetFloodFillDataForTile( northTile, result );

			Tile* southTile = GetTileFromWorldCoords( worldCoords + m_surroundingTilePositions[(int)SurroundingTiles::SOUTH] );
			SetFloodFillDataForTile( southTile, result );

			Tile* eastTile = GetTileFromWorldCoords( worldCoords + m_surroundingTilePositions[(int)SurroundingTiles::EAST] );
			SetFloodFillDataForTile( eastTile, result );

			Tile* westTile = GetTileFromWorldCoords( worldCoords + m_surroundingTilePositions[(int)SurroundingTiles::WEST] );
			SetFloodFillDataForTile( westTile, result );

			result.m_tilesProcessed[tileIndex] = true;
			changesWereMade = true;
		}
	}

	return result;
}


//-----------------------------------------------------------------------------------------------
void Map::SetFloodFillDataForTile( Tile* tile, FloodFillResult& result )
{
	if ( tile != nullptr )
	{
		int tileIndex = GetTileIndexFromTileCoords( tile->m_tileCoords );
		if ( !IsTileSolid( *tile ) )
		{
			result.m_tilesReachable[tileIndex] = true;
		}
	}
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntity( EntityType type, EntityFaction faction, const Vec2& position, float orientationDegrees )
{
	Entity* newEntity = nullptr;
	switch ( type )
	{
		case ENTITY_TYPE_PLAYER: newEntity = new Player( this, faction, position );
			break;
		case ENTITY_TYPE_NPC_TURRET: newEntity = new NpcTurret( this, faction, position );
			break;
		case ENTITY_TYPE_NPC_TANK: newEntity = new NpcTank( this, faction, position );
			break;
		case ENTITY_TYPE_BOULDER: newEntity = new Boulder( this, faction, position );
			break;
		case ENTITY_TYPE_GOOD_BULLET: newEntity = new Bullet( this, type, faction, position, orientationDegrees );
			break;
		case ENTITY_TYPE_EVIL_BULLET: newEntity = new Bullet( this, type, faction, position, orientationDegrees );
			break;
	}

	AddEntityToVector( newEntity, type );

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
void Map::AddEntityToVector( Entity* entity, EntityType vectorType )
{
	if ( vectorType != ENTITY_TYPE_INVALID )
	{
		m_entityVectorsByType[vectorType].push_back( entity );
		if ( vectorType == ENTITY_TYPE_PLAYER )
		{
			m_player = (Player*)entity;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::RemoveEntityFromVector( Entity* entityToRemove, EntityType vectorType )
{
	if ( vectorType != ENTITY_TYPE_INVALID )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[vectorType].size(); ++entityIndex )
		{
			Entity* entity = m_entityVectorsByType[vectorType][entityIndex];
			if ( entity == nullptr )
			{
				continue;
			}

			if ( entity == entityToRemove )
			{
				entityToRemove = nullptr;
			}
		}
	}

	// There can be only 1 player, so if it is removed clear the vector instead of leaving a nullptr
	if ( vectorType == ENTITY_TYPE_PLAYER )
	{
		m_player = nullptr;
		m_entityVectorsByType[ENTITY_TYPE_PLAYER].clear();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DeleteGarbageEntities()
{
	for ( int entityType = 0; entityType < NUM_ENTITY_TYPES; ++entityType )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityType].size(); ++entityIndex )
		{
			Entity*& entity = m_entityVectorsByType[entityType][entityIndex];
			if ( entity == nullptr )
			{
				continue;
			}

			if ( entity->IsGarbage() )
			{
				delete entity;
				entity = nullptr;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::PopulateEnemySpawnLocations( const MapEntityDefinition& mapEntityDefinition )
{
	for ( int tankCount = 0; tankCount < mapEntityDefinition.m_numNPCTanks; ++tankCount )
	{
		AddSpawnPositionToVector( GetRandomValidSpawnPosition(), ENTITY_TYPE_NPC_TANK );
	}
	
	for ( int turretCount = 0; turretCount < mapEntityDefinition.m_numNPCTurrets; ++turretCount )
	{
		AddSpawnPositionToVector( GetRandomValidSpawnPosition(), ENTITY_TYPE_NPC_TURRET );
	}

	for ( int boulderCount = 0; boulderCount < mapEntityDefinition.m_numBoulders; ++boulderCount )
	{
		AddSpawnPositionToVector( GetRandomValidSpawnPosition(), ENTITY_TYPE_BOULDER );
	}
}


//-----------------------------------------------------------------------------------------------
Vec2 Map::GetRandomValidSpawnPosition()
{
	Vec2 spawnPos( -1.f, -1.f);
	 
	while ( IsPointInSolid( spawnPos ) 
			|| IsPointInSafeZone( spawnPos ) )
	{
		spawnPos.x = g_game->m_rng->RollRandomFloatInRange( 0.f, (float)m_width );
		spawnPos.y = g_game->m_rng->RollRandomFloatInRange( 0.f, (float)m_height );
	} 

	return spawnPos;
}


//-----------------------------------------------------------------------------------------------
bool Map::IsPointInSafeZone( const Vec2& position )
{
	Vec2 safeZoneSize( (float)( SAFE_ZONE_SIZE + 2 ), (float)( SAFE_ZONE_SIZE + 2 ) );
	
	AABB2 startZone( Vec2::ZERO, safeZoneSize );
	if ( startZone.IsPointInside( position ) )
	{
		return true;
	}

	Vec2 mapSize( (float)m_width, (float)m_height );
	AABB2 exitZone( mapSize - safeZoneSize, mapSize );
	if ( exitZone.IsPointInside( position ) )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Map::AddSpawnPositionToVector( const Vec2& spawnPosition, EntityType type )
{
	if ( type != ENTITY_TYPE_INVALID )
	{
		m_entitySpawnPositionVectorsByType[type].push_back( spawnPosition );
	}
}


//-----------------------------------------------------------------------------------------------
int Map::GetTileIndexFromTileCoords( int xCoord, int yCoord )
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
int Map::GetTileIndexFromTileCoords( const IntVec2& coords )
{
	return GetTileIndexFromTileCoords( coords.x, coords.y );
}


//-----------------------------------------------------------------------------------------------
int Map::GetTileIndexFromWorldCoords( const Vec2& coords )
{
	return GetTileIndexFromTileCoords( (int)coords.x, (int)coords.y );
}


//-----------------------------------------------------------------------------------------------
Tile* Map::GetTileFromWorldCoords( const Vec2& coords )
{
	int tileIndex = GetTileIndexFromTileCoords( (int)coords.x, (int)coords.y );
	if ( tileIndex < 0 
		 || tileIndex >= (int)m_tiles.size() )
	{
		return nullptr;
	}

	return &( m_tiles[tileIndex] );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Map::GetWorldCoordsFromTile( const Tile& tile )
{
	return Vec2( (float)tile.m_tileCoords.x, (float)tile.m_tileCoords.y );
}

#include "Game/Map.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"

#include "Game/Player.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( int width, int height )
	: m_width( width )
	, m_height( height )
{
	PopulateTiles();
	SpawnPlayer();
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	// For now this will also delete the player
	for ( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityTypeIndex].size(); ++entityIndex )
		{
			Entity*& entity = m_entityVectorsByType[entityTypeIndex][entityIndex];
			delete entity;
			entity = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateEntities( deltaSeconds );
	ResolveCollisions();
	UpdateCameras();
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateEntities( float deltaSeconds )
{
	for ( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityTypeIndex].size(); ++entityIndex )
		{
			Entity*& entity = m_entityVectorsByType[entityTypeIndex][entityIndex];
			if ( entity != nullptr )
			{
				entity->Update( deltaSeconds );
			}
		}
	}
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
void Map::DebugRender() const
{
	DebugRenderEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::RenderTiles() const
{
	for ( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		const Tile& tile = m_tiles[tileIndex];

		g_renderer->BindTexture( nullptr );
		g_renderer->DrawAABB2( tile.GetBounds(), GetColorForTileType( tile.m_tileType ) );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::RenderEntities() const
{
	for ( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityTypeIndex].size(); ++entityIndex )
		{
			Entity*const& entity = m_entityVectorsByType[entityTypeIndex][entityIndex];
			entity->Render();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRenderEntities() const
{
	for ( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityTypeIndex].size(); ++entityIndex )
		{
			Entity*const& entity = m_entityVectorsByType[entityTypeIndex][entityIndex];
			entity->DebugRender();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveCollisions()
{
	for ( int entityTypeIndex = 0; entityTypeIndex < NUM_ENTITY_TYPES; ++entityTypeIndex )
	{
		for ( int entityIndex = 0; entityIndex < (int)m_entityVectorsByType[entityTypeIndex].size(); ++entityIndex )
		{
			Entity* const& entity = m_entityVectorsByType[entityTypeIndex][entityIndex];
			if ( entity == nullptr )
			{
				continue;
			}

			// Check if tile collisions are enabled
			if ( !g_game->IsNoClipEnabled() )
			{
				ResolveEntityCollisionsWithSurroundingTiles( entity );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityCollisions( Entity& a, Entity& b )
{
	if ( a.IsPushedByEntities() && b.IsPushedByEntities() )
	{
		return;
	}

	if ( !a.PushesEntities() && !b.PushesEntities() )
	{
		return;
	}

	/*if ( !DoEntitiesOverlap( a, b ) )
	{
		return;
	}*/

	//if(a.Pushes && b.IsPushedByEntities && !aIsPushedByEntities)
	//{
	//	PushDiscOutOfDisc2D();
	//}
	//else if(b.Pushes && a.IsPushedByEntities && !bIsPushedByEntities)
	//{
	//	PushDiscOutOfDisc2D();
	//}
	//else if(a.Pushes && b.IsPushedByEntities && !aIsPushedByEntities)
	//{
	//	PushDiscOutOfDisc2D();
	//}
	// One more case at least

}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityCollisionsWithSurroundingTiles( Entity* entity )
{
	// Check tiles in following order for collision
	// 5 1 6
	// 4 * 2
	// 8 3 7
	Vec2 tilePosition( 0.f, TILE_SIZE );
	ResolveEntityCollisionWithTile( entity, entity->m_position + tilePosition );

	tilePosition = Vec2( TILE_SIZE, 0.f );
	ResolveEntityCollisionWithTile( entity, entity->m_position + tilePosition );

	tilePosition = Vec2( 0.f, -TILE_SIZE );
	ResolveEntityCollisionWithTile( entity, entity->m_position + tilePosition );

	tilePosition = Vec2( -TILE_SIZE, 0.f );
	ResolveEntityCollisionWithTile( entity, entity->m_position + tilePosition );

	tilePosition = Vec2( -TILE_SIZE, TILE_SIZE );
	ResolveEntityCollisionWithTile( entity, entity->m_position + tilePosition );

	tilePosition = Vec2( TILE_SIZE, TILE_SIZE );
	ResolveEntityCollisionWithTile( entity, entity->m_position + tilePosition );

	tilePosition = Vec2( TILE_SIZE, -TILE_SIZE );
	ResolveEntityCollisionWithTile( entity, entity->m_position + tilePosition );

	tilePosition = Vec2( -TILE_SIZE, -TILE_SIZE );
	ResolveEntityCollisionWithTile( entity, entity->m_position + tilePosition );
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityCollisionWithTile( Entity* entity, Vec2 tilePosition )
{
	int entityTileIndex = GetTileIndexFromTileCoords( (int)tilePosition.x, (int)tilePosition.y );
	if ( entityTileIndex < 0
		|| entityTileIndex >= (int)m_tiles.size() )
	{
		return;
	}

	Tile entityTile = m_tiles[entityTileIndex];
	if ( IsTileTypeSolid( entityTile.m_tileType ) )
	{
		Vec2 newPosition( entity->m_position );
		PushDiscOutOfAABB2D( newPosition, entity->GetPhysicsRadius(), entityTile.GetBounds() );
		entity->m_position = newPosition;
	}
}


//-----------------------------------------------------------------------------------------------
void Map::PopulateTiles()
{
	CreateInitialTiles();
	DrawStoneBorder();
	RandomlyDistributeStone();
	ClearSafeZones();
}


//-----------------------------------------------------------------------------------------------
void Map::CreateInitialTiles()
{
	for ( int yIndex = 0; yIndex < m_height; ++yIndex )
	{
		for ( int xIndex = 0; xIndex < m_width; ++xIndex )
		{
			m_tiles.push_back( Tile( xIndex, yIndex, TileType::TILE_TYPE_GRASS ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DrawStoneBorder()
{
	for ( int xIndex = 0; xIndex < m_width; ++xIndex )
	{
		m_tiles[GetTileIndexFromTileCoords( xIndex, 0 )].m_tileType = TileType::TILE_TYPE_STONE;
		m_tiles[GetTileIndexFromTileCoords( xIndex, m_height - 1 )].m_tileType = TileType::TILE_TYPE_STONE;
	}

	for ( int yIndex = 0; yIndex < m_height; ++yIndex )
	{
		m_tiles[GetTileIndexFromTileCoords( 0, yIndex )].m_tileType = TileType::TILE_TYPE_STONE;
		m_tiles[GetTileIndexFromTileCoords( m_width - 1, yIndex )].m_tileType = TileType::TILE_TYPE_STONE;
	}
}


//-----------------------------------------------------------------------------------------------
void Map::RandomlyDistributeStone()
{
	for ( int yIndex = 0; yIndex < m_height; ++yIndex )
	{
		for ( int xIndex = 0; xIndex < m_width; ++xIndex )
		{
			int stoneThreshhold = g_game->m_rng->RollRandomIntLessThan( 10 );
			if ( stoneThreshhold <= 2 )
			{
				m_tiles[GetTileIndexFromTileCoords( xIndex, yIndex )].m_tileType = TileType::TILE_TYPE_STONE;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ClearSafeZones()
{
	// Bottom Left
	for ( int yIndex = 1; yIndex < SAFE_ZONE_SIZE + 1; ++yIndex )
	{
		for ( int xIndex = 1; xIndex < SAFE_ZONE_SIZE + 1; ++xIndex )
		{
			m_tiles[GetTileIndexFromTileCoords( xIndex, yIndex )].m_tileType = TileType::TILE_TYPE_GRASS;
		}
	}

	// Top Right
	for ( int yIndex = m_height - 2; yIndex > m_height - ( SAFE_ZONE_SIZE + 2 ); --yIndex )
	{
		for ( int xIndex = m_width - 2; xIndex > m_width - ( SAFE_ZONE_SIZE + 2 ); --xIndex )
		{
			m_tiles[GetTileIndexFromTileCoords( xIndex, yIndex )].m_tileType = TileType::TILE_TYPE_GRASS;
		}
	}
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntity( EntityType type, EntityFaction faction, const Vec2& position )
{
	Entity* newEntity = nullptr;
	switch ( type )
	{
		case ENTITY_TYPE_PLAYER:
			newEntity = new Player( faction, position );
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
	}
}


//-----------------------------------------------------------------------------------------------
void Map::SpawnPlayer()
{
	m_player = (Player*)SpawnNewEntity( ENTITY_TYPE_PLAYER, ENTITY_FACTION_GOOD, Vec2( 2.f, 2.f ) );
}


//-----------------------------------------------------------------------------------------------
int Map::GetTileIndexFromTileCoords( int xCoord, int yCoord )
{
	return xCoord + yCoord * m_width;
}

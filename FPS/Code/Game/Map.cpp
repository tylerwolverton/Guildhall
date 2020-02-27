#include "Game/Map.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
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

#include "Game/Actor.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( std::string name, MapDefinition* mapDef )
	: m_name( name )
	, m_mapDef( mapDef )
{
	m_width = mapDef->m_width;
	m_height = mapDef->m_height;

	BuildCardinalDirectionsArray();
	PopulateTiles();
	SpawnPlayer();
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	// For now this will also delete the player
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		delete entity;
		entity = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	UpdateEntities( deltaSeconds );
	ResolveCollisions();
	UpdateCameras();
	UpdateMouseDebugInspection();
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateEntities( float deltaSeconds )
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		if ( entity != nullptr )
		{
			entity->Update( deltaSeconds );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateMouseDebugInspection()
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		if ( entity != nullptr
			 && IsPointInsideDisc( g_game->GetMouseWorldPosition(), entity->GetPosition(), entity->GetPhysicsRadius() ) )
		{
			std::string xPos = Stringf( "x: %.2f", entity->GetPosition().x );
			std::string yPos = Stringf( "y: %.2f", entity->GetPosition().y );

			std::vector< std::string > textLines = { entity->GetName(), xPos, yPos };
			g_game->PrintToDebugInfoBox( Rgba8::WHITE, textLines );
			return;
		}
	}

	// Print out tile name
	Tile* tile = GetTileFromWorldCoords( g_game->GetMouseWorldPosition() );
	if ( tile )
	{
		std::string xPos = Stringf( "x: %d", tile->m_tileCoords.x );
		std::string yPos = Stringf( "y: %d", tile->m_tileCoords.y );

		std::vector< std::string > textLines = { tile->GetName(), xPos, yPos };
		g_game->PrintToDebugInfoBox( Rgba8::WHITE, textLines );
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
		
		//g_game->SetWorldCameraOrthographicView( cameraBounds );
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

		//g_game->SetWorldCameraOrthographicView( cameraBounds );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::BuildCardinalDirectionsArray()
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
void Map::Render() const
{
	//RenderTiles();
	//RenderEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
	DebugRenderEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::RenderTiles() const
{
	std::vector<Vertex_PCU> vertices;

	for ( int tileIndex = 0; tileIndex < m_tiles.size(); ++tileIndex )
	{
		const Tile& tile = m_tiles[tileIndex];

		g_renderer->AppendVertsForAABB2D( vertices, tile.GetBounds(), tile.m_tileDef->GetSpriteTint(), tile.m_tileDef->GetUVCoords().mins, tile.m_tileDef->GetUVCoords().maxs );
	}

	//g_renderer->BindTexture( &(g_tileSpriteSheet->GetTexture()) );
	//g_renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void Map::RenderEntities() const
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*const& entity = m_entities[entityIndex];
		entity->Render();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRenderEntities() const
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*const& entity = m_entities[entityIndex];
		entity->DebugRender();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveCollisions()
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		if ( entity == nullptr )
		{
			continue;
		}

		// Check if tile collisions are enabled
		if ( !g_game->IsNoClipEnabled() )
		{
			ResolveEntityCollisionsWithSurroundingTiles( *entity );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveEntityCollisionsWithSurroundingTiles( Entity& entity )
{
	// Check tiles in following order for collision
	// 5 1 6
	// 4 * 2
	// 8 3 7
	constexpr int numDirections = sizeof( m_cardinalDirectionOffsets ) / sizeof( m_cardinalDirectionOffsets[0] );
	for ( int dirIndex = 1; dirIndex < numDirections; ++dirIndex )
	{
		ResolveEntityCollisionWithTile( entity, entity.m_position + m_cardinalDirectionOffsets[dirIndex] );
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
	if ( !CanEntityEnterTile( entity, entityTile ) )
	{
		Vec2 newPosition( entity.m_position );
		PushDiscOutOfAABB2D( newPosition, entity.GetPhysicsRadius(), entityTile.GetBounds() );
		entity.m_position = newPosition;
	}
}


//-----------------------------------------------------------------------------------------------
bool Map::CanEntityEnterTile( const Entity& entity, const Tile& tile )
{
	if ( entity.CanFly() 
		 && tile.AllowsFlying() )
	{
		return true;
	}

	if ( entity.m_entityDef->CanWalk()
		 && tile.AllowsWalking() )
	{
		return true;
	}

	if ( entity.m_entityDef->CanSwim()
		 && tile.AllowsSwimming() )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Map::PopulateTiles()
{
	CreateInitialTiles();
	SetEdgeTiles();
}


//-----------------------------------------------------------------------------------------------
void Map::CreateInitialTiles()
{
	if ( m_mapDef->m_fillTile == nullptr )
	{
		ERROR_AND_DIE( Stringf( "Map '%s' does not have a fill tile defined!", m_name.c_str() ) );
	}

	for ( int y = 0; y < m_height; ++y )
	{
		for ( int x = 0; x < m_width; ++x )
		{
			m_tiles.push_back( Tile( x, y, m_mapDef->m_fillTile ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::SetEdgeTiles()
{
	if ( m_mapDef->m_edgeTile == nullptr )
	{
		ERROR_AND_DIE( Stringf( "Map '%s' does not have an edge tile defined!", m_name.c_str() ) );
	}

	for ( int x = 0; x < m_width; ++x )
	{
		m_tiles[GetTileIndexFromTileCoords( x, 0 )].SetTileDef( m_mapDef->m_edgeTile );
		m_tiles[GetTileIndexFromTileCoords( x, m_height - 1 )].SetTileDef( m_mapDef->m_edgeTile );
	}

	for ( int y = 0; y < m_height; ++y )
	{
		m_tiles[GetTileIndexFromTileCoords( 0, y )].SetTileDef( m_mapDef->m_edgeTile );
		m_tiles[GetTileIndexFromTileCoords( m_width - 1, y )].SetTileDef( m_mapDef->m_edgeTile );
	}
}


//-----------------------------------------------------------------------------------------------
Actor* Map::SpawnNewActor(  const Vec2& position, std::string actorName )
{
	Actor* newActor = new Actor( position, ActorDefinition::GetActorDefinition( actorName ) );

	m_entities.push_back( newActor );

	return newActor;
}


//-----------------------------------------------------------------------------------------------
void Map::SpawnPlayer()
{
	m_player = SpawnNewActor( Vec2( 2.f, 2.f ), std::string( "Player" ) );
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
Tile* Map::GetTileFromTileCoords( const IntVec2& tileCoords )
{
	return GetTileFromTileCoords( tileCoords.x, tileCoords.y );
}


//-----------------------------------------------------------------------------------------------
Tile* Map::GetTileFromTileCoords( int xCoord, int yCoord )
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
Tile* Map::GetTileFromWorldCoords( const Vec2& worldCoords )
{
	return GetTileFromTileCoords( IntVec2( (int)worldCoords.x, (int)worldCoords.y ) );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Map::GetWorldCoordsFromTile( const Tile& tile )
{
	return Vec2( (float)tile.m_tileCoords.x, (float)tile.m_tileCoords.y );
}


//-----------------------------------------------------------------------------------------------
std::vector<Tile*> Map::GetTilesInRadius( const Tile& centerTile, int radius, bool includeCenterTile )
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

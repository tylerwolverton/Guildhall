#include "Game/Map.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"

#include "Game/Actor.hpp"
#include "Game/Item.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ItemDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( std::string name, MapDefinition* mapDef )
	: m_name( name )
	, m_mapDef( mapDef )
{
	m_width = mapDef->m_width;
	m_height = mapDef->m_height;

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
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateCameras()
{
	if ( g_game->IsDebugCameraEnabled() )
	{
		Vec2 aspectDimensions = Vec2( WINDOW_WIDTH, WINDOW_HEIGHT );
		AABB2 cameraBounds( Vec2( 0.f, 0.f ), aspectDimensions );
		cameraBounds.StretchToIncludePointMaintainAspect( Vec2( (float)m_width, (float)m_height ), aspectDimensions );
		
		g_game->SetWorldCameraPosition( Vec3( cameraBounds.GetCenter(), 0.f ) );
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

		g_game->SetWorldCameraPosition( Vec3( cameraBounds.GetCenter(), 0.f ) );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::Render() const
{
	std::vector<Vertex_PCU> vertexes;
	AppendVertsForAABB2D( vertexes, AABB2( Vec2::ZERO, Vec2( (float)m_width, (float)m_height ) ), Rgba8::WHITE );

	g_renderer->BindTexture( 0, m_mapDef->m_backgroundTexture );
	g_renderer->DrawVertexArray( vertexes );

	RenderEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
	DebugRenderEntities();
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
Actor* Map::SpawnNewActor(  const Vec2& position, std::string actorName )
{
	Actor* newActor = new Actor( position, ActorDefinition::GetActorDefinition( actorName ) );

	m_entities.push_back( newActor );

	return newActor;
}


//-----------------------------------------------------------------------------------------------
Item* Map::SpawnNewItem( const Vec2& position, std::string itemName )
{
	Item* newItem = new Item( position, ItemDefinition::GetItemDefinition( itemName ) );

	m_entities.push_back( newItem );

	return newItem;
}


//-----------------------------------------------------------------------------------------------
void Map::SpawnPlayer()
{
	// TODO: Load position from XML
	m_player = SpawnNewActor( Vec2( 2.f, 1.f ), std::string( "Player" ) );

	Item* key = SpawnNewItem( Vec2( 2.f, 2.f ), std::string( "Key" ) );
	g_game->AddItemToInventory( key );

	Item* key1 = SpawnNewItem( Vec2( 4.f, 2.f ), std::string( "Key" ) );
	g_game->AddItemToInventory( key1 );

	Item* key2 = SpawnNewItem( Vec2( 2.f, 1.f ), std::string( "Key" ) );
	g_game->AddItemToInventory( key2 );

	g_game->RemoveItemFromInventory(key1);
	
	Item* key3 = SpawnNewItem( Vec2( 4.f, 1.f ), std::string( "Key" ) );
	g_game->AddItemToInventory( key3 );

}

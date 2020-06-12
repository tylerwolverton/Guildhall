#include "Game/Map.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/DebugRender.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/Actor.hpp"
#include "Game/Item.hpp"
#include "Game/TriggerRegion.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/ItemDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( std::string name, MapDefinition* mapDef )
	: m_name( name )
	, m_mapDef( mapDef )
{
	//g_eventSystem->RegisterEvent( "VerbAction", "", GAME, &Map::OnVerbAction );
	g_eventSystem->RegisterMethodEvent( "VerbAction", "", GAME, this, &Map::OnVerbAction );

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
	UpdateMouseHover();
	CheckForTriggers();
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
void Map::UpdateMouseHover()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		std::string entityName = entity->GetName();

		if ( entityName != "Player"
			 && IsPointInsideDisc( g_game->GetMouseWorldPosition(), entity->GetPosition(), entity->GetPhysicsRadius() ) )
		{
			Vec2 hintPosition( entity->GetPosition() );
			hintPosition.y += entity->GetPhysicsRadius();
			
			DebugAddWorldTextf( Mat44::CreateTranslation2D( hintPosition ),
								Vec2( .5f, .5f ),
								Rgba8::WHITE,
								0.f,
								0.1f,
								DEBUG_RENDER_ALWAYS,
								entityName.c_str() );
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::CheckForTriggers()
{
	for ( int triggerIndex = 0; triggerIndex < (int)m_triggerRegions.size(); ++triggerIndex )
	{
		TriggerRegion& triggerRegion = m_triggerRegions[triggerIndex];
		if ( DoDiscsOverlap( m_player->GetPosition(), m_player->GetPhysicsRadius(), triggerRegion.GetPosition(), triggerRegion.GetPhysicsRadius() ) )
		{
			triggerRegion.OnTriggerEnter( (Actor*)m_player );
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
void Map::PickUpItem( const Vec2& worldPosition )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		if ( entity->GetName() == "Key"
			 && IsPointInsideDisc( worldPosition, entity->GetPosition(), entity->GetPhysicsRadius() ) )
		{
			g_game->AddItemToInventory( (Item*)entity );
			m_entities[entityIdx] = nullptr;
			return;
		}
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
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Render();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRenderEntities() const
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*const& entity = m_entities[entityIndex];
		if ( entity == nullptr )
		{
			continue;
		}
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

	m_entities = m_mapDef->GetEntitiesInLevel();
	if ( m_entities.size() > 0 )
	{
		m_player = m_entities[0];//(Entity*)ActorDefinition::GetActorDefinition( "Player" );
		//m_player = (Entity*)ActorDefinition::GetActorDefinition( "Player" );
	}
	//Item* key = SpawnNewItem( Vec2( 12.f, 2.f ), std::string( "Key" ) );
	//g_game->AddItemToInventory( key );

	//Item* key1 = SpawnNewItem( Vec2( 4.f, 2.f ), std::string( "Key" ) );
	////g_game->AddItemToInventory( key1 );

	//Item* key2 = SpawnNewItem( Vec2( 2.f, 1.f ), std::string( "Key" ) );
	////g_game->AddItemToInventory( key2 );

	////g_game->RemoveItemFromInventory(key1);
	//
	//Item* key3 = SpawnNewItem( Vec2( 4.f, 1.f ), std::string( "Key" ) );
	////g_game->AddItemToInventory( key3 );

	if ( m_entities.size() > 1 )
	{
		m_triggerRegions.emplace_back( Vec2( 2.f, 0.f ), 1.f, "Victory" );
		m_triggerRegions[0].AddRequiredItem( (Item*)m_entities[1] );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::OnVerbAction( EventArgs* args )
{
	std::string str( "Event received at position: " );
	Vec2 pos = args->GetValue( "Position", Vec2( -1.f, -1.f ) );
	str += pos.ToString();
	g_devConsole->PrintString( str );

	if ( args->GetValue( "Type", std::string( "" ) ) == std::string( "PickUp" ) )
	{
		PickUpItem( pos );
	}
}

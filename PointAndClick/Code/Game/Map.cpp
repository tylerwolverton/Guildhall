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
	g_eventSystem->RegisterMethodEvent( OnPickUpVerbEventName, "", GAME, this, &Map::OnPickupVerb );
	g_eventSystem->RegisterMethodEvent( OnOpenVerbEventName, "", GAME, this, &Map::OnOpenVerb );
	g_eventSystem->RegisterMethodEvent( OnCloseVerbEventName, "", GAME, this, &Map::OnCloseVerb );
	g_eventSystem->RegisterMethodEvent( OnTalkToVerbEventName, "", GAME, this, &Map::OnTalkToVerb );

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

			g_game->SetNounText( entityName );

			return;
		}
	}

	g_game->SetNounText( "" );
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
void Map::SpawnPlayer()
{
	// TODO: Load position from XML

	m_entities = m_mapDef->GetEntitiesInLevel();
	m_items = m_mapDef->GetItemsInLevel();
	
	if ( m_entities.size() > 0 )
	{
		m_player = m_entities[0];
		g_game->SetPlayer( (Actor*)m_player );

		DebugAddWorldTextf( Mat44::CreateTranslation2D( m_player->GetPosition() + Vec2( 2.f, 1.f ) ),
							Vec2( .5f, .5f ),
							Rgba8::WHITE,
							2.f,
							.15f,
							DEBUG_RENDER_ALWAYS,
							"Great, locked in again..." );
	}

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
	Vec2 worldPosition = args->GetValue( "Position", Vec2( -1.f, -1.f ) );
	str += worldPosition.ToString();
	g_devConsole->PrintString( str );

	for ( int itemIdx = 0; itemIdx < (int)m_items.size(); ++itemIdx )
	{
		Item*& item = m_items[itemIdx];
		if ( item == nullptr )
		{
			continue;
		}

		if ( IsPointInsideDisc( worldPosition, item->GetPosition(), item->GetPhysicsRadius() ) )
		{
			int verbStateInt = args->GetValue( "Type", (int)eVerbState::NONE );
			if ( verbStateInt < (int)eVerbState::NONE
				 || verbStateInt >( int )eVerbState::LAST_VAL )
			{
				g_devConsole->PrintError( Stringf( "Verb type '%d' is not defined", verbStateInt ) );
				return;
			}

			item->HandleVerbAction( eVerbState( verbStateInt ) );
		}
	}

	/*if ( args->GetValue( "Type", std::string( "" ) ) == std::string( "PickUp" ) )
	{
		PickUpItem( pos );
	}*/
}


//-----------------------------------------------------------------------------------------------
void Map::OnPickupVerb( EventArgs* args )
{
	Item* targetItem = (Item*)args->GetValue( "target", ( void* )nullptr );

	if ( targetItem == nullptr )
	{
		g_devConsole->PrintError( "Tried to pickup an item but target was null" );
		return;
	}

	g_game->AddItemToInventory( targetItem );

	// Remove from map since game now owns the item
	for ( int itemIdx = 0; itemIdx < (int)m_items.size(); ++itemIdx )
	{
		Item*& item = m_items[itemIdx];
		if ( item == nullptr )
		{
			continue;
		}

		if ( item == targetItem )
		{
			m_items[itemIdx] = nullptr;
		}
	}

	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		if ( entity == targetItem )
		{
			m_entities[entityIdx] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::OnOpenVerb( EventArgs* args )
{
	Item* targetItem = (Item*)args->GetValue( "target", ( void* )nullptr );

	if ( targetItem == nullptr )
	{
		g_devConsole->PrintError( "Tried to open an item but target was null" );
		return;
	}

	Texture* openTexture = (Texture*)args->GetValue( "texture", ( void* )nullptr );
	targetItem->SetTexture( openTexture );
}


//-----------------------------------------------------------------------------------------------
void Map::OnCloseVerb( EventArgs* args )
{
	Item* targetItem = (Item*)args->GetValue( "target", ( void* )nullptr );

	if ( targetItem == nullptr )
	{
		g_devConsole->PrintError( "Tried to close an item but target was null" );
		return;
	}
	
	Texture* closedTexture = (Texture*)args->GetValue( "texture", ( void* )nullptr );
	targetItem->SetTexture( closedTexture );
}


//-----------------------------------------------------------------------------------------------
void Map::OnTalkToVerb( EventArgs* args )
{
	Item* targetItem = (Item*)args->GetValue( "target", ( void* )nullptr );

	if ( targetItem == nullptr )
	{
		g_devConsole->PrintError( "Tried to talk to someone but target was null" );
		return;
	}

	g_devConsole->PrintString( Stringf( "Talked to: '%s'", targetItem->GetName().c_str() ), Rgba8::PURPLE );
}

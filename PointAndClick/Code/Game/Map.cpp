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
#include "Game/DialogueState.hpp"
#include "Game/World.hpp"
#include "Game/Actor.hpp"
#include "Game/Item.hpp"
#include "Game/Portal.hpp"
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
	m_width = mapDef->m_width;
	m_height = mapDef->m_height;

	SpawnEntities();
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	// Don't delete the player or cursor
	m_entities.pop_back();
	m_entities.pop_back();

	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		if ( entity == nullptr )
		{
			continue;
		}

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
	if ( g_game->GetGameState() == eGameState::PLAYING )
	{
		UpdateMouseHover();
	}

	CheckForTriggers();
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateEntities( float deltaSeconds )
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		if ( entity == nullptr
			 || entity->GetName() == "Cursor" )
		{
			continue;
		}
		
		entity->Update( deltaSeconds );
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
			 && entity->GetType() != "portal"
			 && IsPointInsideDisc( g_game->GetMouseWorldPosition(), entity->GetPosition(), entity->GetPhysicsRadius() ) )
		{
			Vec2 hintPosition( entity->GetPosition() );
			hintPosition.y += entity->GetTopOfSprite();
			
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
	/*for ( int triggerIndex = 0; triggerIndex < (int)m_triggerRegions.size(); ++triggerIndex )
	{
		TriggerRegion& triggerRegion = m_triggerRegions[triggerIndex];
		if ( DoDiscsOverlap( m_player->GetPosition(), m_player->GetPhysicsRadius(), triggerRegion.GetPosition(), triggerRegion.GetPhysicsRadius() ) )
		{
			triggerRegion.OnTriggerEnter( (Actor*)m_player );
		}
	}*/
	for ( int portalIdx = 0; portalIdx < (int)m_portals.size(); ++portalIdx )
	{
		Portal*& portal = m_portals[portalIdx];
		if ( portal == nullptr
			 || !portal->IsActive() )
		{
			continue;
		}

		if ( DoDiscsOverlap( m_player->GetPosition(), m_player->GetPhysicsRadius(), portal->GetPosition(), portal->GetPhysicsRadius() ) )
		{
			portal->OnEnter( (Actor*)m_player );
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
void Map::TiePortalToDoor()
{
	// Tie portal to forest to door state
	Item* door = nullptr;
	Portal* portalToForest = nullptr;
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity* const& entity = m_entities[entityIndex];
		if ( entity == nullptr )
		{
			continue;
		}

		if ( entity->GetName() == "Door" )
		{
			door = (Item*)entity;
		}
		else if ( entity->GetName() == "PortalToForest" )
		{
			portalToForest = (Portal*)entity;
		}
	}

	if ( door != nullptr
		 && portalToForest != nullptr )
	{
		if ( door->IsOpen() )
		{
			portalToForest->Activate();
		}
		else
		{
			portalToForest->Deactivate();
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
void Map::Load( Entity* player, Cursor* cursor )
{
	g_eventSystem->RegisterMethodEvent( "VerbAction", "", GAME, this, &Map::OnVerbAction );
	g_eventSystem->RegisterMethodEvent( OnPickUpVerbEventName, "", GAME, this, &Map::OnPickupVerb );
	g_eventSystem->RegisterMethodEvent( OnOpenVerbEventName, "", GAME, this, &Map::OnOpenVerb );
	g_eventSystem->RegisterMethodEvent( OnCloseVerbEventName, "", GAME, this, &Map::OnCloseVerb );
	g_eventSystem->RegisterMethodEvent( OnTalkToVerbEventName, "", GAME, this, &Map::OnTalkToVerb );
	g_eventSystem->RegisterMethodEvent( OnGiveToSourceVerbEventName, "", GAME, this, &Map::OnGiveToSourceVerb );
	g_eventSystem->RegisterMethodEvent( OnGiveToDestinationVerbEventName, "", GAME, this, &Map::OnGiveToDestinationVerb );

	m_player = player;
	m_entities.push_back( player );
	m_entities.push_back( (Entity*)cursor );

	m_player->SetPosition( m_mapDef->GetPlayerStartPos() );
	((Actor*)m_player)->SetMoveTargetLocation( m_mapDef->GetPlayerStartPos() );

	TiePortalToDoor();
}


//-----------------------------------------------------------------------------------------------
void Map::Unload()
{
	g_eventSystem->DeRegisterObject( this );

	m_entities.pop_back();
	m_entities.pop_back();
	m_player = nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetEntityByName( const std::string& name )
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		if ( entity != nullptr 
			 && entity->GetName() == name )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void Map::RenderEntities() const
{
	int numEntitiesToRender = (int)m_entities.size();

	// Could build a dynamic system that keeps track of each entity rendered and orders them while avoiding nullptrs...
	// Or could just do this since we only have 2 layers
	for ( int entityIndex = 0; entityIndex < numEntitiesToRender; ++entityIndex )
	{
		Entity* const& entity = m_entities[entityIndex];
		if ( entity == nullptr
			 || entity->GetName() == "Cursor" )
		{
			continue;
		}

		if ( entity->GetDrawOrder() == 0 )
		{
			entity->Render();
		}
	}

	for ( int entityIndex = 0; entityIndex < numEntitiesToRender; ++entityIndex )
	{
		Entity* const& entity = m_entities[entityIndex];
		if ( entity == nullptr
			 || entity->GetName() == "Cursor" )
		{
			continue;
		}

		if ( entity->GetDrawOrder() == 1 )
		{
			entity->Render();
		}
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
void Map::SpawnEntities()
{
	m_entities = m_mapDef->GetEntitiesInLevel();
	m_items = m_mapDef->GetItemsInLevel();
	m_portals = m_mapDef->GetPortalsInLevel();
	TiePortalToDoor();
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
	targetItem->RemoveVerbState( eVerbState::PICKUP );

	NamedProperties* properties = new NamedProperties();
	properties->SetValue( "eventName", GetEventNameForVerbState( eVerbState::GIVE_TO_SOURCE ) );
	
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
	NamedProperties* props = (NamedProperties*)args->GetValue( "properties", ( void* )nullptr );

	if ( targetItem == nullptr )
	{
		g_devConsole->PrintError( "Tried to open an item but target was null" );
		return;
	}

	if ( targetItem->IsOpen() )
	{
		g_game->PrintTextOverPlayer( "It's already open." );
		return;
	}

	std::string requiredItemName = props->GetValue( "requiredItem", "" );

	if ( !g_game->IsItemInInventory( requiredItemName ) )
	{
		g_game->PrintTextOverPlayer( "It's locked tight." );
		return;
	}

	Texture* openTexture = (Texture*)props->GetValue( "texture", ( void* )nullptr );
	targetItem->SetTexture( openTexture );
	targetItem->Open();

	TiePortalToDoor();
}


//-----------------------------------------------------------------------------------------------
void Map::OnCloseVerb( EventArgs* args )
{
	Item* targetItem = (Item*)args->GetValue( "target", ( void* )nullptr );
	NamedProperties* props = (NamedProperties*)args->GetValue( "properties", ( void* )nullptr );

	if ( targetItem == nullptr )
	{
		g_devConsole->PrintError( "Tried to close an item but target was null" );
		return;
	}
	
	if ( !targetItem->IsOpen() )
	{
		g_game->PrintTextOverPlayer( "It's already closed." );
		return;
	}

	Texture* closedTexture = (Texture*)props->GetValue( "texture", ( void* )nullptr );
	targetItem->SetTexture( closedTexture );
	targetItem->Close();

	TiePortalToDoor();
}


//-----------------------------------------------------------------------------------------------
void Map::OnTalkToVerb( EventArgs* args )
{
	Item* target = (Item*)args->GetValue( "target", ( void* )nullptr );
	NamedProperties* props = (NamedProperties*)args->GetValue( "properties", ( void* )nullptr );

	if ( target == nullptr )
	{
		g_devConsole->PrintError( "Tried to talk to someone but target was null" );
		return;
	}

	std::string initialDialogueStateName = props->GetValue( "initialDialogueState", "" );

	DialogueState* initialState = DialogueState::GetDialogueState( initialDialogueStateName );
	if ( initialState != nullptr )
	{
		g_game->BeginConversation( initialState, target );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::OnGiveToSourceVerb( EventArgs* args )
{
	Item* targetItem = (Item*)args->GetValue( "target", ( void* )nullptr );

	if ( targetItem == nullptr )
	{
		g_devConsole->PrintError( "Tried to give an item but target was null" );
		return;
	}

	g_game->SetNounText( targetItem->GetName() );
	g_game->SetPlayerVerbState( eVerbState::GIVE_TO_DESTINATION );
}


//-----------------------------------------------------------------------------------------------
void Map::OnGiveToDestinationVerb( EventArgs* args )
{
	Item* targetItem = (Item*)args->GetValue( "target", ( void* )nullptr );
	NamedProperties* props = (NamedProperties*)args->GetValue( "properties", ( void* )nullptr );
	
	if ( targetItem == nullptr )
	{
		g_devConsole->PrintError( "Tried to give an item but target was null" );
		return;
	}

	std::string acceptedItemName = props->GetValue( "acceptedItem", "" );

	if ( g_game->IsItemInInventory( acceptedItemName ) 
		 && g_game->GetNounText() == acceptedItemName )
	{
		g_game->RemoveItemFromInventory( acceptedItemName );

		std::string receivedItemName = props->GetValue( "receivedItem", "" );
		if ( receivedItemName != ""  )
		{
			ItemDefinition* itemDef = ItemDefinition::GetItemDefinition( receivedItemName );
			if ( itemDef != nullptr )
			{
				g_game->AddItemToInventory( new Item( Vec2::ZERO, itemDef ) );
			}
		}

		std::string text = props->GetValue( "text", "" );
		g_game->PrintTextOverEntity( *targetItem, text, 2.f );

		if ( targetItem->GetName() == "Cook"
			 && acceptedItemName == "Rock" )
		{
			for ( int itemIdx = 0; itemIdx < (int)m_items.size(); ++itemIdx )
			{
				Item*& item = m_items[itemIdx];
				if ( item == nullptr )
				{
					continue;
				}

				if ( item->GetName() == receivedItemName )
				{
					item->SetIsInPlayerInventory( true );
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

				if ( entity->GetName() == receivedItemName )
				{
					m_entities[entityIdx] = nullptr;
				}
			}
		}
	}
	else
	{
		std::string text = props->GetValue( "failText", "" );
		g_game->PrintTextOverEntity( *targetItem, text, 2.f );
	}
}

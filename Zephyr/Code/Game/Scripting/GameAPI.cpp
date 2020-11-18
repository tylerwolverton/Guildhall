#include "Game/Scripting/GameAPI.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRender.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Entity.hpp"


#define REGISTER_EVENT( eventName ) {\
										m_registeredMethods.insert( #eventName );\
										g_eventSystem->RegisterMethodEvent( #eventName, "", EVERYWHERE, this, &GameAPI::eventName );\
									}

//-----------------------------------------------------------------------------------------------
GameAPI::GameAPI()
{
	REGISTER_EVENT( ChangeZephyrScriptState );
	REGISTER_EVENT( PrintDebugText );
	REGISTER_EVENT( PrintDebugScreenText );
	REGISTER_EVENT( PrintToConsole );

	REGISTER_EVENT( DestroySelf );
	REGISTER_EVENT( StartDialogue );
	REGISTER_EVENT( EndDialogue );
	REGISTER_EVENT( AddLineOfDialogueText );
	REGISTER_EVENT( AddDialogueChoice );
	REGISTER_EVENT( StartNewTimer );
	REGISTER_EVENT( WinGame );

	REGISTER_EVENT( MoveToLocation );
	REGISTER_EVENT( ChaseTargetEntity );
	REGISTER_EVENT( FleeTargetEntity );
	REGISTER_EVENT( GetEntityLocation );
	REGISTER_EVENT( CheckForTarget );
	REGISTER_EVENT( GetNewWanderTargetPosition );
	REGISTER_EVENT( GetDistanceToTarget );

	REGISTER_EVENT( SpawnEntity );
	REGISTER_EVENT( DamageEntity );
	REGISTER_EVENT( ActivateInvincibility );
	REGISTER_EVENT( DeactivateInvincibility );
	REGISTER_EVENT( AddNewDamageTypeMultiplier );
	REGISTER_EVENT( ChangeDamageTypeMultiplier );

	REGISTER_EVENT( ChangeSpriteAnimation );
	REGISTER_EVENT( PlaySound );
	REGISTER_EVENT( ChangeMusic );
	REGISTER_EVENT( AddScreenShake );
}


//-----------------------------------------------------------------------------------------------
GameAPI::~GameAPI()
{
	m_registeredMethods.clear();
}


//-----------------------------------------------------------------------------------------------
bool GameAPI::IsMethodRegistered( const std::string& methodName )
{
	auto iter = m_registeredMethods.find( methodName );
	
	return iter != m_registeredMethods.end();
}


//-----------------------------------------------------------------------------------------------
void GameAPI::DestroySelf( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity != nullptr )
	{
		entity->Die();
	}
}


//-----------------------------------------------------------------------------------------------
void GameAPI::DamageEntity( EventArgs* args )
{
	EntityId entityId = (EntityId)args->GetValue( "id", 0.f );
	float damage = args->GetValue( "damage", 0.f );
	std::string damageType = args->GetValue( "damageType", "normal" );

	Entity* entityToDamage = g_game->GetEntityById( entityId );

	if ( entityToDamage != nullptr )
	{
		entityToDamage->TakeDamage( damage, damageType );
	}
}


//-----------------------------------------------------------------------------------------------
void GameAPI::ActivateInvincibility( EventArgs* args )
{
	Entity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	entity->MakeInvincibleToAllDamage();
}


//-----------------------------------------------------------------------------------------------
void GameAPI::DeactivateInvincibility( EventArgs* args )
{
	Entity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	entity->ResetDamageMultipliers();
}


//-----------------------------------------------------------------------------------------------
void GameAPI::AddNewDamageTypeMultiplier( EventArgs* args )
{
	Entity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	float newDamageMultiplier = args->GetValue( "multiplier", 1.f );
	std::string type = args->GetValue( "damageType", "" );
	if ( type.empty() )
	{
		return;
	}

	entity->AddNewDamageMultiplier( type, newDamageMultiplier );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::ChangeDamageTypeMultiplier( EventArgs* args )
{
	Entity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	float newDamageMultiplier = args->GetValue( "multiplier", 1.f );
	std::string type = args->GetValue( "damageType", "" );
	if ( type.empty() )
	{
		return;
	}

	entity->ChangeDamageMultiplier( type, newDamageMultiplier );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::StartDialogue( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::DIALOGUE );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::EndDialogue( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::PLAYING );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::AddLineOfDialogueText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );

	g_game->AddLineOfDialogueText( text );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::AddDialogueChoice( EventArgs* args )
{
	std::string name = args->GetValue( "name", "missing name" );
	std::string text = args->GetValue( "text", "" );

	g_game->AddDialogueChoice( name, text );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::StartNewTimer( EventArgs* args )
{
	std::string timerName = args->GetValue( "name", "" );
	float durationSeconds = args->GetValue( "durationSeconds", 1.f );
	std::string onCompletedEventName = args->GetValue( "onCompletedEvent", "" );
	bool broadcastEventToAll = args->GetValue( "broadcastEventToAll", false );
	std::string targetName = args->GetValue( "targetName", "" );

	// Broadcast event to all takes precedence and broadcasts to all entities
	if ( broadcastEventToAll )
	{
		g_game->StartNewTimer( -1, timerName, durationSeconds, onCompletedEventName );
		return;
	}

	// Named target takes precedence over sending it to self
	if ( !targetName.empty() )
	{
		g_game->StartNewTimer( targetName, timerName, durationSeconds, onCompletedEventName );
		return;
	}

	// Send event to entity who fired it
	EntityId targetId = -1;
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	if ( entity != nullptr )
	{
		targetId = entity->GetId();
	}

	g_game->StartNewTimer( targetId, timerName, durationSeconds, onCompletedEventName );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::ChangeZephyrScriptState( EventArgs* args )
{
	std::string targetState = args->GetValue( "targetState", "" );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity != nullptr
		 && !targetState.empty() )
	{
		entity->ChangeZephyrScriptState( targetState );
	}
}


//-----------------------------------------------------------------------------------------------
void GameAPI::PrintDebugText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "TestPrint" );
	float duration = args->GetValue( "duration", 0.f );
	Entity* entity = (Entity*)args->GetValue( "entity", (void*)nullptr );

	std::string colorStr = args->GetValue( "color", "white" );

	Rgba8 color = Rgba8::WHITE;
	if ( colorStr == "white" )		{ color = Rgba8::WHITE; }
	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
	else if ( colorStr == "blue" )  { color = Rgba8::BLUE; }
	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }

	Mat44 textLocation;

	if ( entity != nullptr )
	{
		textLocation.SetTranslation2D( entity->GetPosition() );
	}
	
	DebugAddWorldText( textLocation, Vec2::HALF, color, color, duration, .1f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::PrintDebugScreenText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );
	float duration = args->GetValue( "duration", 0.f );
	float fontSize = args->GetValue( "fontSize", 24.f );
	Vec2 locationRatio = args->GetValue( "locationRatio", Vec2::ZERO );
	Vec2 padding = args->GetValue( "padding", Vec2::ZERO );

	std::string colorStr = args->GetValue( "color", "white" );

	Rgba8 color = Rgba8::WHITE;
	if ( colorStr == "white" )		{ color = Rgba8::WHITE; }
	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
	else if ( colorStr == "blue" )	{ color = Rgba8::BLUE; }
	else if ( colorStr == "black" )	{ color = Rgba8::BLACK; }
	
	DebugAddScreenText( Vec4( locationRatio, padding ), Vec2::ZERO, fontSize, color, color, duration, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::PrintToConsole( EventArgs* args )
{
	std::string text = args->GetValue( "text", "TestPrint" );
	std::string colorStr = args->GetValue( "color", "white" );

	Rgba8 color = Rgba8::WHITE;
	if		( colorStr == "white" ) { color = Rgba8::WHITE; }
	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
	else if ( colorStr == "green" )	{ color = Rgba8::GREEN; }
	else if ( colorStr == "blue" )	{ color = Rgba8::BLUE; }
	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }
	
	g_devConsole->PrintString( text.c_str(), color );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::SpawnEntity( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	if ( entity == nullptr )
	{
		return;
	}

	std::string entityType = args->GetValue( "type", "" );
	std::string mapName = args->GetValue( "map", "" );
	Vec2 position = args->GetValue( "position", entity->GetPosition() );

	Map* mapToSpawnIn = entity->GetMap();
	if ( !mapName.empty() )
	{
		mapToSpawnIn = g_game->GetMapByName( mapName );
		if ( mapToSpawnIn == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Can't spawn entity in nonexistent map '%s'", mapName.c_str() ) );
			return;
		}
	}

	Entity* newEntity = mapToSpawnIn->SpawnNewEntityOfTypeAtPosition( entityType, position );
	newEntity->FireSpawnEvent();
	if ( mapToSpawnIn == g_game->GetCurrentMap() )
	{
		newEntity->Load();
	}
}


//-----------------------------------------------------------------------------------------------
void GameAPI::WinGame( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::VICTORY );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::MoveToLocation( EventArgs* args )
{
	Vec2 targetPos = args->GetValue( "pos", Vec2::ZERO );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr )
	{
		return;
	}

	Vec2 moveDirection = targetPos - entity->GetPosition();
	moveDirection.Normalize();

	float moveSpeed = entity->GetWalkSpeed() * g_game->GetLastDeltaSecondsf();

	entity->MoveWithPhysics( moveSpeed, moveDirection );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::ChaseTargetEntity( EventArgs* args )
{
	std::string targetId = args->GetValue( "id", "" );
	Entity* targetEntity = g_game->GetEntityByName( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr 
		 || targetEntity == nullptr )
	{
		return;
	}

	Vec2 moveDirection = targetEntity->GetPosition() - entity->GetPosition();
	moveDirection.Normalize();

	float moveSpeed = entity->GetWalkSpeed() * g_game->GetLastDeltaSecondsf();

	entity->MoveWithPhysics( moveSpeed, moveDirection );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::FleeTargetEntity( EventArgs* args )
{
	std::string targetId = args->GetValue( "id", "" );
	Entity* targetEntity = g_game->GetEntityByName( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	Vec2 moveDirection = targetEntity->GetPosition() - entity->GetPosition();
	moveDirection.Normalize();

	float moveSpeed = entity->GetWalkSpeed() * g_game->GetLastDeltaSecondsf();

	entity->MoveWithPhysics( moveSpeed, -moveDirection );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::GetEntityLocation( EventArgs* args )
{
	std::string targetId = args->GetValue( "id", "" );
	Entity* targetEntity = g_game->GetEntityByName( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	EventArgs targetArgs;
	targetArgs.SetValue( "entityPos", targetEntity->GetPosition() );

	entity->FireScriptEvent( "UpdateEntityLocation", &targetArgs );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::GetNewWanderTargetPosition( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	if ( entity == nullptr )
	{
		return;
	}

	float newX = g_game->m_rng->RollRandomFloatInRange( 2.f, 10.f );
	float newY = g_game->m_rng->RollRandomFloatInRange( 2.f, 7.f );
	
	EventArgs targetArgs;
	targetArgs.SetValue( "newPos", Vec2( newX, newY ) );

	entity->FireScriptEvent( "UpdateTargetPosition", &targetArgs );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::CheckForTarget( EventArgs* args )
{
	std::string targetName = args->GetValue( "id", "" );
	float maxDist = args->GetValue( "maxDist", 0.f );
	Entity* targetEntity = g_game->GetEntityByName( targetName );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	Vec2 displacement = targetEntity->GetPosition() - entity->GetPosition();
	float distBetween = displacement.GetLength();
	if ( distBetween < maxDist )
	{
		EventArgs targetArgs;
		targetArgs.SetValue( "targetName", targetName );

		entity->FireScriptEvent( "TargetFound", &targetArgs );
	}
}


//-----------------------------------------------------------------------------------------------
void GameAPI::GetDistanceToTarget( EventArgs* args )
{
	std::string targetId = args->GetValue( "id", "" );
	Entity* targetEntity = g_game->GetEntityByName( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	Vec2 displacementToTarget = targetEntity->GetPosition() - entity->GetPosition();

	EventArgs targetArgs;
	targetArgs.SetValue( "distance", displacementToTarget.GetLength() );

	entity->FireScriptEvent( "UpdateDistanceToTarget", &targetArgs );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::ChangeSpriteAnimation( EventArgs* args )
{
	std::string newAnim = args->GetValue( "newAnim", "" );
	//Entity* targetEntity = g_game->GetEntityByName( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || newAnim.empty() )
	{
		return;
	}

	entity->ChangeSpriteAnimation( newAnim );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::PlaySound( EventArgs* args )
{
	std::string soundName = args->GetValue( "soundName", "" );
	if ( soundName.empty() )
	{
		return;
	}

	float volume = args->GetValue( "volume", 1.f );
	float balance = args->GetValue( "balance", 0.f );
	float speed = args->GetValue( "speed", 1.f );

	g_game->PlaySoundByName( soundName, false, volume, balance, speed );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::ChangeMusic( EventArgs* args )
{
	std::string musicName = args->GetValue( "musicName", "" );
	if ( musicName.empty() )
	{
		return;
	}

	float volume = args->GetValue( "volume", 1.f );
	float balance = args->GetValue( "balance", 0.f );
	float speed = args->GetValue( "speed", 1.f );

	g_game->ChangeMusic( musicName, true, volume, balance, speed );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::AddScreenShake( EventArgs* args )
{
	float intensity = args->GetValue( "intensity", 0.f );

	g_game->AddScreenShakeIntensity( intensity );
}


//-----------------------------------------------------------------------------------------------
Entity* GameAPI::GetTargetEntityFromArgs( EventArgs* args )
{
	EntityId targetId = (EntityId)args->GetValue( "targetId", -1.f );
	std::string targetName = args->GetValue( "targetName", "" );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	// Named entities are returned first
	if ( !targetName.empty() )
	{
		entity = g_game->GetEntityByName( targetName );
		if ( entity == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Failed to find an entity with name '%s'", targetName.c_str() ) );
			return nullptr;
		}
	}
	// Id entities are tried next
	else if ( targetId != -1 )
	{
		entity = g_game->GetEntityById( targetId );
		if ( entity == nullptr )
		{
			g_devConsole->PrintWarning( Stringf( "Failed to find an entity with id '%i'", targetId ) );
			return nullptr;
		}
	}

	// If no name or id defined, send back the entity who called the method
	return entity;
}


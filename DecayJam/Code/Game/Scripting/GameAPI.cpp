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
	REGISTER_EVENT( MoveInDirection );
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
	REGISTER_EVENT( SpawnNewEntityInInventory );
	REGISTER_EVENT( AddItemToInventory );
	REGISTER_EVENT( RemoveItemFromInventory );
	REGISTER_EVENT( ThrowItemFromInventory );

	REGISTER_EVENT( RegisterKeyEvent );
	REGISTER_EVENT( UnRegisterKeyEvent );

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
/**
 * Destroys the entity who called this event.
 */
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
/**
 * Damages target entity
 * 
 * params:
 *	- id: id of entity to damage
 *		- Zephyr type: Number
 *	- damage: amount of damage to deal
 *		- Zephyr type: Number
 *	- damageType: type of damage to deal
 *		- Zephyr type: String
 *		- default: "normal"
*/
//-----------------------------------------------------------------------------------------------
void GameAPI::DamageEntity( EventArgs* args )
{
	EntityId targetId = (EntityId)args->GetValue( "id", -1.f );
	float damage = args->GetValue( "damage", 0.f );
	std::string damageType = args->GetValue( "damageType", "normal" );

	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	Entity* entityToDamage = g_game->GetEntityById( targetId );

	if ( entityToDamage != nullptr 
		 && entityToDamage->GetFaction() != entity->GetFaction() )
	{
		entityToDamage->TakeDamage( damage, damageType );
	}
}


//-----------------------------------------------------------------------------------------------
/**
 * Makes target entity invincible to all damage entity. 
 * 
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event. 
 * params:
 *	- targetId: id of entity to damage
 *		- Zephyr type: Number
 *	- targetName: amount of damage to deal
 *		- Zephyr type: String
*/
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
/**
 * Reset target entity's damage multipliers.
 *
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 * params:
 *	- targetId: id of entity to damage
 *		- Zephyr type: Number
 *	- targetName: amount of damage to deal
 *		- Zephyr type: String
*/
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
/**
 * Add a new damage type multiplier to target entity. 
 *
 * Description: When the entity takes damage the type of damage will be used to apply a multiplier if one exists for the given type. If an unregistered type of damage is dealt it will be applied at 1x. 
 *
 * params:
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 *	- targetId: id of entity to damage
 *		- Zephyr type: Number
 *	- targetName: amount of damage to deal
 *		- Zephyr type: String
 *	
 *	- multiplier: the multiplier to apply to entity upon taking damage of the given type
 *		- Zephyr type: Number
 *	- damageType: the name of the damage type to register
 *		- Zephyr type: String
*/
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
/**
 * Change the damage type multiplier for a given damage type for target entity.
 *
 * params:
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 *	- targetId: id of entity to damage
 *		- Zephyr type: Number
 *	- targetName: amount of damage to deal
 *		- Zephyr type: String
 *
 *	- multiplier: the multiplier to apply to entity upon taking damage of the given type
 *		- Zephyr type: Number
 *	- damageType: the name of the damage type to register
 *		- Zephyr type: String
*/
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
void GameAPI::SpawnNewEntityInInventory( EventArgs* args )
{
	Entity* newEntity = SpawnEntityFromArgs( args );
	Entity* targetEntity = GetTargetEntityFromArgs( args );

	if ( newEntity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	targetEntity->AddItemToInventory( newEntity );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::AddItemToInventory( EventArgs* args )
{
	Entity* itemEntity = GetItemEntityFromArgs( args );
	Entity* targetEntity = GetTargetEntityFromArgs( args );

	if ( itemEntity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	targetEntity->AddItemToInventory( itemEntity );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::RemoveItemFromInventory( EventArgs* args )
{
	Entity* itemEntity = GetItemEntityFromArgs( args );
	Entity* targetEntity = GetTargetEntityFromArgs( args );

	if ( itemEntity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	targetEntity->RemoveItemFromInventory( itemEntity );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::ThrowItemFromInventory( EventArgs* args )
{
	Entity* targetEntity = GetTargetEntityFromArgs( args );
	std::string itemType = args->GetValue( "itemType", "" );
	if ( targetEntity == nullptr 
		 || itemType.empty() )
	{
		return;
	}

	std::vector<Entity*> inventory = targetEntity->GetInventory();
	Entity* itemToThrow = nullptr;
	for ( int itemIdx = 0; itemIdx < (int)inventory.size(); ++itemIdx )
	{
		if ( inventory[itemIdx] != nullptr
			 && inventory[itemIdx]->GetType() == itemType )
		{
			itemToThrow = inventory[itemIdx];
			break;
		}
	}

	if ( itemToThrow == nullptr )
	{
		return;
	}
	
	EventArgs itemArgs;
	itemToThrow->FireScriptEvent( "ItemThrown", &itemArgs );

	/*Vec2 direction = args->GetValue( "direction", Vec2::ZERO );
	if ( direction == Vec2::ZERO )
	{
		return;
	}

	direction.Normalize();

	float speed = args->GetValue( "speed", itemToThrow->GetSpeed() );*/

	targetEntity->RemoveItemFromInventory( itemToThrow );

	Map* mapToSpawnIn = targetEntity->GetMap();
	if ( mapToSpawnIn == nullptr )
	{
		mapToSpawnIn = g_game->GetCurrentMap();
	}

	mapToSpawnIn->TakeOwnershipOfEntity( itemToThrow );
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the game state to Dialogue to initiate a dialogue with an NPC.
 */
//-----------------------------------------------------------------------------------------------
void GameAPI::StartDialogue( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::DIALOGUE );
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the game state back to Playing to end a dialogue sequence.
 */
//-----------------------------------------------------------------------------------------------
void GameAPI::EndDialogue( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::PLAYING );
}


//-----------------------------------------------------------------------------------------------
/**
 * Add a line of text to the current dialogue box. 
 *	- Note: Only works if StartDialogue has been called first.
 *
 * params:
 *	- text: Text to add to dialogue box
 *		- Zephyr type: String
 */
//-----------------------------------------------------------------------------------------------
void GameAPI::AddLineOfDialogueText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );

	g_game->AddLineOfDialogueText( text );
}


//-----------------------------------------------------------------------------------------------
/**
 * Add a player selectable choice to the current dialogue box. The player can then select choices with the wasd or arrow keys and confirm with space/enter.
 *	- Note: Only works if StartDialogue has been called first.
 *
 * params:
 *	- name: Name to identify this choice in order to assign a callback event on selection
 *		- Zephyr type: String
 *		- A script can listen for the event "PlayerInteracted" which has a "String choiceName" parameter containing the name of the choice that was selected
 *	
 *	- text: Text to add for choice to dialogue box
 *		- Zephyr type: String
 */
//-----------------------------------------------------------------------------------------------
void GameAPI::AddDialogueChoice( EventArgs* args )
{
	std::string name = args->GetValue( "name", "missing name" );
	std::string text = args->GetValue( "text", "" );

	g_game->AddDialogueChoice( name, text );
}


//-----------------------------------------------------------------------------------------------
/**
 * Start a new timer to fire an event on completion.
 *
 * params:
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 *	- targetId: id of entity to damage
 *		- Zephyr type: Number
 *	- targetName: amount of damage to deal
 *		- Zephyr type: String
 *
 *	- name: the name of the timer ( currently unused, reserved for future use  )
 *		- Zephyr type: String
 *	- durationSeconds: length of time in seconds of timer
 *		- Zephyr type: Number
 *	- onCompletedEvent: the name of the event to fire upon timer completion
 *		- Zephyr type: String
 *	- broadcastEventToAll: when true, broadcast the onCompletedEvent to all entities ( this will override any target entity set )
 *		- Zephyr type: Bool
 *		- default: false
*/
//-----------------------------------------------------------------------------------------------
void GameAPI::StartNewTimer( EventArgs* args )
{
	std::string timerName = args->GetValue( "name", "" );
	float durationSeconds = args->GetValue( "durationSeconds", 1.f );
	std::string onCompletedEventName = args->GetValue( "onCompletedEvent", "" );
	bool broadcastEventToAll = args->GetValue( "broadcastEventToAll", false );
	
	// Broadcast event to all takes precedence and broadcasts to all entities
	if ( broadcastEventToAll )
	{
		g_game->StartNewTimer( -1, timerName, durationSeconds, onCompletedEventName );
		return;
	}
	
	Entity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	g_game->StartNewTimer( entity->GetId(), timerName, durationSeconds, onCompletedEventName );
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the current State of the Zephyr script for the entity who called the event.
 *	Note: Called like ChangeState( newState ) in script as a built in function
 *
 * params:
 *	- targetState: the name of the Zephyr State to change to
 *	- Zephyr type: String
*/
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
/**
 * Print debug world text at position of entity who called the event.
 *
 * params:
 *	- text: text to print
 *		- Zephyr type: String
 *	- duration: duration in seconds to display text
 *		- Zephyr type: Number
 *		- default: 0 ( single frame )
 *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
 *		- Zephyr type: String
 *		- default: "white"
*/
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
/**
 * Print debug screen text at given position.
 *
 * params:
 *	- text: text to print
 *		- Zephyr type: String
 *	- duration: duration in seconds to display text
 *		- Zephyr type: Number
 *		- default: 0 ( single frame )
 *	- fontSize: height in pixels of font 
 *		- Zephyr type: Number 
 *		- default: 24
 *	- locationRatio: position of font on screen, x and y are between 0 and 1
 *		- Zephyr type: Vec2
 *		- default: ( 0, 0 )
 *	- padding: how much padding in pixels to add to text position
 *		- Zephyr type: Vec2
 *		- default: ( 0, 0 )
 *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
 *		- Zephyr type: String
 *		- default: "white"
*/
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
/**
 * Print text to dev console.
 *
 * params:
 *	- text: text to print
 *		- Zephyr type: String
 *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
 *		- Zephyr type: String
 *		- default: "white"
*/
//-----------------------------------------------------------------------------------------------
void GameAPI::PrintToConsole( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );
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
	SpawnEntityFromArgs( args );
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

	float speed = args->GetValue( "speed", entity->GetSpeed() );

	entity->MoveWithPhysics( speed, moveDirection );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::MoveInDirection( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	if ( entity == nullptr )
	{
		return;
	}

	Vec2 direction = args->GetValue( "direction", Vec2::ZERO );
	if ( direction == Vec2::ZERO )
	{
		return;
	}

	direction.Normalize();

	float speed = args->GetValue( "speed", entity->GetSpeed() );

	entity->MoveWithPhysics( speed, direction );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::ChaseTargetEntity( EventArgs* args )
{
	Entity* targetEntity = GetTargetEntityFromArgs( args );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	
	if ( entity == nullptr 
		 || targetEntity == nullptr 
		 || entity == targetEntity )
	{
		return;
	}

	Vec2 moveDirection = targetEntity->GetPosition() - entity->GetPosition();
	moveDirection.Normalize();

	float speed = args->GetValue( "speed", entity->GetSpeed() );

	entity->MoveWithPhysics( speed, moveDirection );
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

	float speed = args->GetValue( "speed", entity->GetSpeed() );

	entity->MoveWithPhysics( speed, -moveDirection );
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

	entity->FireScriptEvent( "EntityLocationUpdated", &targetArgs );
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

	entity->FireScriptEvent( "TargetPositionUpdated", &targetArgs );
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

	entity->FireScriptEvent( "DistanceToTargetUpdated", &targetArgs );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::RegisterKeyEvent( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	if ( entity == nullptr )
	{
		return;
	}

	std::string key = args->GetValue( "key", "" );
	if ( key.empty() )
	{
		return;
	}

	std::string event = args->GetValue( "event", "" );
	if ( event.empty() )
	{
		return;
	}

	entity->RegisterKeyEvent( key, event );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::UnRegisterKeyEvent( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	if ( entity == nullptr )
	{
		return;
	}

	std::string key = args->GetValue( "key", "" );
	if ( key.empty() )
	{
		return;
	}

	std::string event = args->GetValue( "event", "" );
	if ( event.empty() )
	{
		return;
	}

	entity->UnRegisterKeyEvent( key, event );
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
Entity* GameAPI::SpawnEntityFromArgs( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	if ( entity == nullptr )
	{
		return nullptr;
	}

	std::string entityType = args->GetValue( "type", "" );
	if ( entityType.empty() )
	{
		return nullptr;
	}

	std::string name = args->GetValue( "name", "" );
	std::string mapName = args->GetValue( "map", "" );
	Vec2 position = args->GetValue( "position", entity->GetPosition() );
	float orientation = args->GetValue( "orientation", entity->GetOrientationDegrees() );

	Map* mapToSpawnIn = entity->GetMap();
	if ( mapToSpawnIn == nullptr )
	{
		mapToSpawnIn = g_game->GetCurrentMap();
	}

	if ( !mapName.empty() )
	{
		mapToSpawnIn = g_game->GetMapByName( mapName );
		if ( mapToSpawnIn == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Can't spawn entity in nonexistent map '%s'", mapName.c_str() ) );
			return nullptr;
		}
	}

	Entity* newEntity = mapToSpawnIn->SpawnNewEntityOfTypeAtPosition( entityType, position );
	newEntity->SetOrientationDegrees( orientation );
	newEntity->SetName( name );
	g_game->SaveEntityByName( newEntity );
	newEntity->FireSpawnEvent();
	if ( mapToSpawnIn == g_game->GetCurrentMap() )
	{
		newEntity->Load();
	}

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
Entity* GameAPI::GetItemEntityFromArgs( EventArgs* args )
{
	EntityId itemId = (EntityId)args->GetValue( "itemId", -1.f );
	std::string itemName = args->GetValue( "itemName", "" );
	Entity* entity = nullptr;

	// Named entities are returned first
	if ( !itemName.empty() )
	{
		entity = g_game->GetEntityByName( itemName );
		if ( entity == nullptr )
		{
			g_devConsole->PrintWarning( Stringf( "Failed to find an entity with name '%s'", itemName.c_str() ) );
			return nullptr;
		}
	}
	// Id entities are tried next
	else if ( itemId != -1 )
	{
		entity = g_game->GetEntityById( itemId );
		if ( entity == nullptr )
		{
			g_devConsole->PrintWarning( Stringf( "Failed to find an entity with id '%i'", itemId ) );
			return nullptr;
		}
	}
	else
	{
		g_devConsole->PrintError( Stringf( "No item name or id specified" ) );
		return nullptr;
	}

	return entity;
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


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
	REGISTER_EVENT( PrintToConsole );

	REGISTER_EVENT( DestroySelf );
	REGISTER_EVENT( SpawnEntity );
	REGISTER_EVENT( DamageEntity );
	REGISTER_EVENT( UpdateEnemyCount );
	REGISTER_EVENT( WinGame );

	REGISTER_EVENT( MoveToLocation );
	REGISTER_EVENT( ChaseTargetEntity );
	REGISTER_EVENT( FleeTargetEntity );
	REGISTER_EVENT( GetEntityLocation );
	REGISTER_EVENT( CheckForTarget );
	REGISTER_EVENT( GetNewWanderTargetPosition );
	REGISTER_EVENT( GetDistanceToTarget );

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
void GameAPI::UpdateEnemyCount( EventArgs* args )
{
	float enemyCount = args->GetValue( "enemyCount", 0.f );

	g_game->UpdateEnemyCount( (int)enemyCount );
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

	Entity* entityToDamage = g_game->GetEntityById( entityId );

	if ( entityToDamage != nullptr )
	{
		entityToDamage->TakeDamage( damage );
	}
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
	Rgba8 color = args->GetValue( "color", Rgba8::WHITE );
	Entity* entity = (Entity*)args->GetValue( "entity", (void*)nullptr );

	Mat44 textLocation;

	if ( entity != nullptr )
	{
		textLocation.SetTranslation2D( entity->GetPosition() );
	}
	
	DebugAddWorldText( textLocation, Vec2::HALF, color, color, duration, .1f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, text.c_str() );
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
		// Find map by name and spawn there
	}

	mapToSpawnIn->SpawnNewEntityOfTypeAtPosition( entityType, position );
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
void GameAPI::AddScreenShake( EventArgs* args )
{
	float intensity = args->GetValue( "intensity", 0.f );

	g_game->AddScreenShakeIntensity( intensity );
}

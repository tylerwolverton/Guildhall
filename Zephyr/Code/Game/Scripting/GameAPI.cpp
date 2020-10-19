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
	REGISTER_EVENT( UpdateEnemyCount );
	REGISTER_EVENT( DestroyEntity );
	REGISTER_EVENT( WinGame );

	REGISTER_EVENT( MoveToLocation );
	REGISTER_EVENT( GetNewWanderTargetPosition );
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
void GameAPI::DestroyEntity( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity != nullptr )
	{
		entity->Die();
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
	targetArgs.SetValue( "newPosX", newX);
	targetArgs.SetValue( "newPosY", newY );

	entity->FireScriptEvent( "UpdateTargetPosition", &targetArgs );
}

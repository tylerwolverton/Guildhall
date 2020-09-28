#include "Game/Scripting/GameAPI.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"
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
	REGISTER_EVENT( UpdateEnemyCount );
	REGISTER_EVENT( EntityBirthEvent );
	REGISTER_EVENT( EntityDeathEvent );
	REGISTER_EVENT( TestResponseEvent );
	REGISTER_EVENT( PrintDebugText );
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
void GameAPI::EntityBirthEvent( EventArgs* args )
{
	UNUSED( args );

	g_eventSystem->FireEvent( "EnemySpawned" );
	//g_game->IncrementEnemyCount();
}


//-----------------------------------------------------------------------------------------------
void GameAPI::EntityDeathEvent( EventArgs* args )
{
	UNUSED( args );

	g_eventSystem->FireEvent( "EnemyDied" );

	//g_game->DecrementEnemyCount();
}


//-----------------------------------------------------------------------------------------------
void GameAPI::TestResponseEvent( EventArgs* args )
{
	UNUSED( args );

	g_devConsole->PrintString( "TestResponse fired", Rgba8::CYAN );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::PrintDebugText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "TestPrint" );
	Entity* entity = (Entity*)args->GetValue( "entity", (void*)nullptr );

	Mat44 textLocation;

	if ( entity != nullptr )
	{
		textLocation.SetTranslation2D( entity->GetPosition() );
	}
	
	DebugAddWorldText( textLocation, Vec2::HALF, Rgba8::WHITE, Rgba8::WHITE, 0.f, .1f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, text.c_str() );
}

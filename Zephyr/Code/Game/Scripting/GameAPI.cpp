#include "Game/Scripting/GameAPI.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/DebugRender.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


#define REGISTER_EVENT( eventName ) {\
										m_registeredMethods.insert( #eventName );\
										g_eventSystem->RegisterMethodEvent( #eventName, "", EVERYWHERE, this, &GameAPI::eventName );\
									}

//-----------------------------------------------------------------------------------------------
GameAPI::GameAPI()
{
	// TODO: Find a clever way to register all methods in this file
	//m_registeredMethods.insert( "EntityBirthEvent" );
	//g_eventSystem->RegisterMethodEvent( "EntityBirthEvent", "", EVERYWHERE, this, &GameAPI::EntityBirthEvent );

	//m_registeredMethods.insert( "EntityDeathEvent" );
	//g_eventSystem->RegisterMethodEvent( "EntityDeathEvent", "", EVERYWHERE, this, &GameAPI::EntityDeathEvent );

	//m_registeredMethods.insert( "TestResponseEvent" );
	//g_eventSystem->RegisterMethodEvent( "TestResponseEvent", "", EVERYWHERE, this, &GameAPI::TestResponseEvent );

	REGISTER_EVENT( EntityBirthEvent );
	REGISTER_EVENT( EntityDeathEvent );
	REGISTER_EVENT( TestResponseEvent );
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
void GameAPI::EntityBirthEvent( EventArgs* args )
{
	UNUSED( args );

	g_game->IncrementEnemyCount();
}


//-----------------------------------------------------------------------------------------------
void GameAPI::EntityDeathEvent( EventArgs* args )
{
	UNUSED( args );

	g_game->DecrementEnemyCount();
}


//-----------------------------------------------------------------------------------------------
void GameAPI::TestResponseEvent( EventArgs* args )
{
	UNUSED( args );

	g_devConsole->PrintString( "TestResponse fired", Rgba8::CYAN );
}


//-----------------------------------------------------------------------------------------------
void GameAPI::PrintDebugTextEvent( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );

	DebugAddScreenText( Vec4( 2.f, 2.f, 0.f, 0.f ), Vec2::ONE, 10.f, Rgba8::WHITE, Rgba8::WHITE, 0.f, text.c_str() );
}

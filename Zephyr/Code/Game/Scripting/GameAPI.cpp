#include "Game/Scripting/GameAPI.hpp"
#include "Engine/Core/EventSystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
GameAPI::GameAPI()
{
	m_registeredMethods.insert( "EntityBirthEvent" );
	g_eventSystem->RegisterMethodEvent( "EntityBirthEvent", "", EVERYWHERE, this, &GameAPI::EntityBirthEvent );

	m_registeredMethods.insert( "EntityDeathEvent" );
	g_eventSystem->RegisterMethodEvent( "EntityDeathEvent", "", EVERYWHERE, this, &GameAPI::EntityDeathEvent );
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


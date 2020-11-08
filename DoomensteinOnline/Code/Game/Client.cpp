#include "Game/Client.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerClient.hpp"
#include "Game/RemoteClient.hpp"


//-----------------------------------------------------------------------------------------------
void Client::Startup()
{
}


//-----------------------------------------------------------------------------------------------
void Client::Shutdown()
{
}


//-----------------------------------------------------------------------------------------------
void Client::BeginFrame()
{

}


//-----------------------------------------------------------------------------------------------
void Client::Update()
{

}


//-----------------------------------------------------------------------------------------------
void Client::Render( const World* gameWorld ) const
{
	UNUSED( gameWorld );
}


//-----------------------------------------------------------------------------------------------
void Client::SetPlayer( Entity* entity )
{
	UNUSED( entity );

	g_devConsole->PrintError( "Cannot set player on a non player client" );
}


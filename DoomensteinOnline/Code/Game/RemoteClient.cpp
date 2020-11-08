#include "Game/RemoteClient.hpp"
#include "Engine/Core/DevConsole.hpp"


//-----------------------------------------------------------------------------------------------
void RemoteClient::Startup()
{
	// Some config to talk to server
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::Shutdown()
{

}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SetClientId( int id )
{
	// Send a message to RemoteServer to set player client's id
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SetPlayer( Entity* entity )
{
	UNUSED( entity );

	g_devConsole->PrintError( "Cannot set player on a non player client" );
}

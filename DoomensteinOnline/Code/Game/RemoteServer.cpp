#include "Game/RemoteServer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Game/SinglePlayerGame.hpp"
#include "Game/MultiplayerGame.hpp"


//-----------------------------------------------------------------------------------------------
RemoteServer::RemoteServer( EventArgs* args )
	: Server( args )
{
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::Startup( eAppMode appMode )
{
	NegotiateUDPConnection();

	StartGame( appMode );
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::Shutdown()
{
	Server::Shutdown();
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::StartGame( eAppMode appMode )
{
	switch ( appMode )
	{
		case eAppMode::SINGLE_PLAYER:
		{
			ERROR_AND_DIE( "Cannot start a Remote server in a single player game" );
		}
		break;

		case eAppMode::MULTIPLAYER_SERVER:
		{
			ERROR_AND_DIE( "Cannot start a Remote server as the main server in multiplayer mode" );
		}
		break;

		case eAppMode::HEADLESS_SERVER:
		{
			ERROR_AND_DIE( "Cannot start a Remote server in headless mode" );
		}
		break;

		case eAppMode::MULTIPLAYER_CLIENT:
		{
			g_game = new MultiplayerGame();
		}
		break;
	}

	g_game->Startup();
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::NegotiateUDPConnection()
{
	EventArgs args;
	args.SetValue( "host", m_ipAddress + ":" + ToString( m_tcpPort ) );
	g_eventSystem->FireEvent( "connect", &args, eUsageLocation::DEV_CONSOLE );
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::Update()
{
	// Copy state of AuthoritativeServer to game
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ReceiveClientRequests( const std::vector<ClientRequest*> clientRequests )
{
	UNUSED( clientRequests );
}


#include "Game/RemoteServer.hpp"
#include "Game/SinglePlayerGame.hpp"
#include "Game/MultiplayerGame.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
void RemoteServer::Startup( eAppMode appMode )
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
void RemoteServer::Shutdown()
{
	Server::Shutdown();
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


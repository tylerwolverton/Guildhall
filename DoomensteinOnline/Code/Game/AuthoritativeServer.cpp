#include "Game/AuthoritativeServer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Game/GameEvents.hpp"
#include "Game/SinglePlayerGame.hpp"
#include "Game/MultiplayerGame.hpp"
#include "Game/PlayerClient.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
AuthoritativeServer::AuthoritativeServer( EventArgs* args )
	: Server( args )
{
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Startup( eAppMode appMode )
{
	StartGame( appMode );
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Shutdown()
{
	Server::Shutdown();
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::StartGame( eAppMode appMode )
{
	switch ( appMode )
	{
		case eAppMode::SINGLE_PLAYER:
		{
			g_game = new SinglePlayerGame();
		}
		break;

		case eAppMode::MULTIPLAYER_SERVER:
		case eAppMode::HEADLESS_SERVER:
		{
			g_game = new MultiplayerGame();
			
			StartTCPServer();
		}
		break;

		case eAppMode::MULTIPLAYER_CLIENT:
		{
			ERROR_AND_DIE( "Cannot start an Authoritative server as a MultiplayerClient" );
		}
		break;
	}

	g_game->Startup();
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessNetworkMessages()
{

}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::StartTCPServer()
{
	EventArgs args;
	args.SetValue( "port", m_tcpPort );
	g_eventSystem->FireEvent( "start_tcp_server", &args, eUsageLocation::DEV_CONSOLE );
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Update()
{
	g_game->Update();

	m_playerClient->Update();

	// Update all remote clients
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::ReceiveClientRequests( const std::vector<ClientRequest*> clientRequests )
{
	for ( int reqIdx = 0; reqIdx < (int)clientRequests.size(); ++reqIdx )
	{
		ClientRequest* const & req = clientRequests[reqIdx];

		if ( req == nullptr )
		{
			continue;
		}

		switch ( req->functionType )
		{
			case eClientFunctionType::CREATE_ENTITY:	
			{
				Entity* newEntity = g_game->CreateEntityInCurrentMap( ( (CreateEntityRequest*)req )->entityType, ( (CreateEntityRequest*)req )->position, ( (CreateEntityRequest*)req )->yawOrientationDegrees );
				if ( newEntity != nullptr
					 && ( (CreateEntityRequest*)req )->entityType == "player" )
				{
					// send player's id back to client and have client possess entity
					req->player = newEntity;
					newEntity->Possess();
				}
			}
			break;

			case eClientFunctionType::POSSESS_ENTITY:				g_game->PossessEntity( req->player, ((PossessEntityRequest*)req)->cameraTransform ); break;
			case eClientFunctionType::UNPOSSESS_ENTITY:				g_game->UnpossessEntity( req->player ); break;
			case eClientFunctionType::SET_PLAYER_ORIENTATION:		g_game->SetPlayerOrientation( req->player, ((SetPlayerOrientationRequest*)req)->yawOrientationDegrees ); break;
			case eClientFunctionType::MOVE_PLAYER:					g_game->MovePlayer( req->player, ((MovePlayerRequest*)req)->translationVec ); break;
		}
	}
}


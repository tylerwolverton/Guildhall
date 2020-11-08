#include "Game/AuthoritativeServer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Networking/NetworkingSystem.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Game/GameEvents.hpp"
#include "Game/SinglePlayerGame.hpp"
#include "Game/MultiplayerGame.hpp"
#include "Game/Client.hpp"
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
	ProcessTCPMessages();
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessTCPMessages()
{
	//if ( m_tcpClientSocket == nullptr )
	//{
	//	return;
	//}

	//TCPData data = m_tcpClientSocket->Receive();
	//if ( data.GetData() == nullptr )
	//{
	//	return;
	//}

	//// Process message
	//const MessageHeader* header = reinterpret_cast<const MessageHeader*>( data.GetData() );
	//switch ( header->id )
	//{
	//	case (uint16_t)eMessasgeProtocolIds::TEXT:
	//	{
	//		const char* dataStr = data.GetData() + 4;
	//		g_devConsole->PrintString( Stringf( "Received from remote server: %s", dataStr ) );
	//	}
	//	break;

	//	case (uint16_t)eMessasgeProtocolIds::DATA:
	//	{

	//	}
	//	break;

	//	default:
	//	{
	//		g_devConsole->PrintError( Stringf( "Received msg with unknown id: %i", header->id ) );
	//		return;
	//	}
	//	break;
	//}
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

	// This server's player client will always be first
	for ( Client* client : m_clients )
	{
		client->Update();
	}
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
				CreateEntityRequest* createEntityReq = (CreateEntityRequest*)req;
				Entity* newEntity = g_game->CreateEntityInCurrentMap( createEntityReq->entityType, createEntityReq->position, createEntityReq->yawOrientationDegrees );
				if ( newEntity != nullptr
					 && createEntityReq->entityType == eEntityType::PLAYER )
				{
					// send player's id back to client and have client possess entity
					m_clients[req->clientId]->SetPlayer( newEntity );
					newEntity->Possess();
				}
			}
			break;

			/*case eClientFunctionType::POSSESS_ENTITY:				g_game->PossessEntity( req->player, ((PossessEntityRequest*)req)->cameraTransform ); break;
			case eClientFunctionType::UNPOSSESS_ENTITY:				g_game->UnpossessEntity( req->player ); break;*/
			case eClientFunctionType::UPDATE_ENTITY:				
			{
				UpdateEntityRequest* updateEntityReq = (UpdateEntityRequest*)req;
				g_game->MoveEntity( updateEntityReq->entityId, updateEntityReq->translationVec );
				g_game->SetEntityOrientation( updateEntityReq->entityId, updateEntityReq->yawOrientationDegrees );
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::RegisterNewClient( Client* client )
{
	int nextClientId = (int)m_clients.size();

	m_clients.push_back( client );

	client->SetClientId( nextClientId );
}



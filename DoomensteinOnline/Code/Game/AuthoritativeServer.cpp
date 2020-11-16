#include "Game/AuthoritativeServer.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/MAth/RandomNumberGenerator.hpp"
#include "Engine/Networking/NetworkingSystem.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Game/GameEvents.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/SinglePlayerGame.hpp"
#include "Game/MultiplayerGame.hpp"
#include "Game/Client.hpp"
#include "Game/PlayerClient.hpp"
#include "Game/RemoteClient.hpp"
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
			
			g_networkingSystem->StartTCPServer( m_tcpPort );
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
	ProcessUDPMessages();
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessTCPMessages()
{
	std::vector<TCPData> newMessages = g_networkingSystem->ReceiveTCPMessages();

	for ( TCPData& data : newMessages )
	{
		if ( data.GetData() == nullptr )
		{
			continue;
		}

		const ClientRequest* req = reinterpret_cast<const ClientRequest*>( data.GetPayload() );
		switch ( req->functionType )
		{
			case eClientFunctionType::REQUEST_CONNECTION:
			{
				int clientKey = m_rng.RollRandomIntInRange( 0, INT_MAX );
				//int udpPort = 48490;// m_rng.RollRandomIntInRange( 48500, 49200 );
				int udpDistantSendToPort = m_rng.RollRandomIntInRange( 48500, 49200 );
				int udpLocalBindPort = 49210;
				//int udpLocalBindPort = m_rng.RollRandomIntInRange( 49210, 49700 );

				ResponseToConnectionRequest response( -1, clientKey, udpDistantSendToPort, udpLocalBindPort, (uint16_t)data.GetFromIPAddress().size() );
				 
				std::array<char, 256> buffer;
				
				memcpy( &buffer[0], &response, sizeof( response ) );
				memcpy( &buffer[sizeof( response )], data.GetFromIPAddress().c_str(), response.size );

				ConnectionInfo info( clientKey, data.GetFromIPAddress(), udpLocalBindPort, udpDistantSendToPort );
				m_clientConnectionInfo.push_back( info );

				g_devConsole->PrintString( Stringf( "Sending connection info: distantSendToPort '%i', localBindPort '%i'", udpDistantSendToPort, udpLocalBindPort ) );

				g_networkingSystem->SendTCPMessage( &buffer, sizeof( response ) + response.size );

				if ( m_clients.size() < 2 )
				{
					g_networkingSystem->OpenAndBindUDPPort( udpLocalBindPort, udpDistantSendToPort );
					g_networkingSystem->CreateAndRegisterUDPSocket( udpDistantSendToPort );
				}
				else
				{
					g_networkingSystem->CreateAndRegisterUDPSocket( udpDistantSendToPort );
				}
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::ProcessUDPMessages()
{
	std::vector<UDPData> newMessages = g_networkingSystem->ReceiveUDPMessages();

	for ( UDPData& data : newMessages )
	{
		if ( data.GetData() == nullptr )
		{
			continue;
		}

		const ClientRequest* req = reinterpret_cast<const ClientRequest*>( data.GetPayload() );
		switch ( req->functionType )
		{
			case eClientFunctionType::KEY_VERIFICATION:
			{
				const KeyVerificationRequest* keyVerifyReq = reinterpret_cast<const KeyVerificationRequest*>( data.GetPayload() );
				
				int foundIdx = -1;
				for ( int connectionIdx = 0; connectionIdx < (int)m_clientConnectionInfo.size(); ++connectionIdx )
				{
					ConnectionInfo info = m_clientConnectionInfo[connectionIdx];
					info.localBindPort = data.GetFromPort();

					if ( info.key == keyVerifyReq->connectKey )
						// && info.ipAddress == data.GetFromIPAddress() )
					{
						RemoteClient* client = new RemoteClient( info );
						RegisterNewClient( client );
						client->Startup();

						foundIdx = connectionIdx;
						break;
					}
				}

				if ( foundIdx >= 0 )
				{
					m_clientConnectionInfo.erase( m_clientConnectionInfo.begin() + foundIdx );
				}
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::Update()
{

	// This server's player client will always be first
	// Gather input from remote servers
	for ( Client* client : m_clients )
	{
		client->Update();
	}

	g_game->Update();

	// After the world has updated, send new state to servers
	/*for ( Client* client : m_clients )
	{
		client->SendUpdatedGameWorldToServer();
	}*/
}


//-----------------------------------------------------------------------------------------------
void AuthoritativeServer::ReceiveClientRequests( const std::vector<const ClientRequest*> clientRequests )
{
	for ( int reqIdx = 0; reqIdx < (int)clientRequests.size(); ++reqIdx )
	{
		const ClientRequest* const & req = clientRequests[reqIdx];

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
				if ( newEntity == nullptr )
				{
					break;
				}
					  
				if( createEntityReq->entityType == eEntityType::PLAYER )
				{
					// send player's id back to client and have client possess entity
					m_clients[req->clientId]->SetPlayer( newEntity );
					newEntity->Possess();
				}

				createEntityReq->entityId = newEntity->GetId();

				// If create was requested, send the new message out to all clients aside from the one requesting it
				for ( auto& client : m_clients )
				{
					if ( client->GetClientId() != req->clientId )
					{
						client->SendMessageToDistantClient( createEntityReq );
					}
				}
			}
			break;

			/*case eClientFunctionType::POSSESS_ENTITY:				g_game->PossessEntity( req->player, ((PossessEntityRequest*)req)->cameraTransform ); break;
			case eClientFunctionType::UNPOSSESS_ENTITY:				g_game->UnpossessEntity( req->player ); break;*/
			case eClientFunctionType::UPDATE_ENTITY:				
			{
				UpdateEntityRequest* updateEntityReq = (UpdateEntityRequest*)req;
				g_game->MoveEntity( updateEntityReq->entityId, updateEntityReq->translationVec );
				g_game->RotateEntity( updateEntityReq->entityId, updateEntityReq->yawRotationDegrees );
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



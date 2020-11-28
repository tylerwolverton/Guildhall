#include "Game/RemoteServer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Networking/NetworkingSystem.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/UDPSocket.hpp"
#include "Game/SinglePlayerGame.hpp"
#include "Game/MultiplayerGame.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameEvents.hpp"
#include "Game/PlayerClient.hpp"


//-----------------------------------------------------------------------------------------------
RemoteServer::RemoteServer( EventArgs* args )
	: Server( args )
{
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::Startup( eAppMode appMode )
{
	UNUSED( appMode );

	RequestUDPConnection();

	//StartGame( appMode );
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::Shutdown()
{
	Server::Shutdown();
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::BeginFrame()
{
	 ProcessNetworkMessages();
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
	g_game->DeleteAllEntities();
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ProcessNetworkMessages()
{
	if ( !m_isTCPClientClosed )
	{
		ProcessTCPMessages();
	}

	ProcessUDPMessages();
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ProcessTCPMessages()
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
			case eClientFunctionType::RESPONSE_TO_CONNECTION_REQUEST:
			{
				const ResponseToConnectionRequest* responseReq = reinterpret_cast<const ResponseToConnectionRequest*>( data.GetPayload() );
				const char* dataStr = data.GetPayload() + sizeof( ResponseToConnectionRequest );
				std::string ipAddress( dataStr );
				ipAddress[responseReq->size] = '\0';

				g_devConsole->PrintString( Stringf( "Response: key = '%i', distantBindPort '%i', localSendToPort '%i', ip = '%s'", responseReq->connectKey, responseReq->distantBindPort, responseReq->localSendToPort, ipAddress.c_str() ) );

				g_networkingSystem->DisconnectTCPClient();
				m_isTCPClientClosed = true;

				m_udpSendToPort = responseReq->localSendToPort;
				g_networkingSystem->OpenAndBindUDPPort( responseReq->distantBindPort, responseReq->localSendToPort );
				g_networkingSystem->CreateAndRegisterUDPSocket( responseReq->localSendToPort );

				KeyVerificationRequest keyVerifyReq( m_remoteClientId, responseReq->connectKey );
				g_networkingSystem->SendUDPMessage( m_udpSendToPort, &keyVerifyReq, sizeof( keyVerifyReq ), true );
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ProcessUDPMessages()
{
	std::vector<UDPData>& newMessages = g_networkingSystem->ReceiveUDPMessages();

	for ( UDPData& data : newMessages )
	{
		if ( data.GetData() == nullptr )
		{
			continue;
		}

		const ClientRequest* req = reinterpret_cast<const ClientRequest*>( data.GetPayload() );
		switch ( req->functionType )
		{
			case eClientFunctionType::REMOTE_CLIENT_REGISTRATION:
			{
				if ( m_remoteClientId == -1 )
				{
					m_remoteClientId = req->clientId;

					StartGame( eAppMode::MULTIPLAYER_CLIENT );

					g_playerClient = new PlayerClient();
					g_server->RegisterNewClient( g_playerClient );
					g_playerClient->Startup();
				}

				data.Process();
			}
			break;

			case eClientFunctionType::SET_PLAYER_ID:
			{
				const SetPlayerIdRequest* setPlayerIdReq = reinterpret_cast<const SetPlayerIdRequest*>( data.GetPayload() );
				
				if ( m_playerClient->GetPlayerId() == -1 )
				{
					m_playerClient->SetPlayerId( setPlayerIdReq->playerId );
				}

				SetPlayerIdAckRequest ackReq( m_remoteClientId );
				g_networkingSystem->SendUDPMessage( m_udpSendToPort, &ackReq, sizeof( ackReq ), true );

				data.Process();
			}
			break;

			case eClientFunctionType::CREATE_ENTITY:
			{
				if ( g_game == nullptr )
				{
					data.Process();
					break;
				}

				const CreateEntityRequest* createEntityReq = reinterpret_cast<const CreateEntityRequest*>( data.GetPayload() );
				Entity* newEntity = g_game->CreateEntityInCurrentMap( createEntityReq->entityId, createEntityReq->entityType, createEntityReq->position, createEntityReq->yawOrientationDegrees );
				if ( newEntity == nullptr )
				{
					data.Process();
					break;
				}
				
				//&& createEntityReq->entityType == eEntityType::PLAYER
				if( createEntityReq->entityId == m_playerClient->GetPlayerId() )
				{
					// send player's id back to client and have client possess entity
					m_playerClient->SetPlayer( newEntity );
					newEntity->Possess();
				}

				data.Process();
			}
			break;

			case eClientFunctionType::UPDATE_ENTITY_ON_REMOTE_SERVER:
			{
				if ( g_game == nullptr )
				{
					data.Process();
					break;
				}

				const UpdateEntityOnRemoteServerRequest* updateEntityReq = reinterpret_cast<const UpdateEntityOnRemoteServerRequest*>( data.GetPayload() );
				g_game->SetEntityPosition( updateEntityReq->entityId, updateEntityReq->positionVec );
				g_game->SetEntityOrientation( updateEntityReq->entityId, updateEntityReq->yawOrientationDegrees );

				data.Process();
			}
			break;

			case eClientFunctionType::SERVER_LAST_DELTA_SECONDS:
			{
				ServerLastDeltaSecondsRequest* serverLastDeltaSecondsReq = (ServerLastDeltaSecondsRequest*)req;
				g_game->SetServerLastDeltaSeconds( serverLastDeltaSecondsReq->deltaSeconds );
			}
			break;

			case eClientFunctionType::NOTIFY_ENTITY_DIED:
			{
				NotifyEntityDiedRequest* notifyDiedReq = (NotifyEntityDiedRequest*)req;
				Entity* entity = g_game->GetEntityById( notifyDiedReq->entityId );
				if ( entity != nullptr )
				{
					entity->Die();
				}
			}
			break;

			/*case eClientFunctionType::UPDATE_ENTITY:
			{
				UpdateEntityRequest* updateEntityReq = (UpdateEntityRequest*)req;
				g_game->MoveEntity( updateEntityReq->entityId, updateEntityReq->translationVec );
				g_game->RotateEntity( updateEntityReq->entityId, updateEntityReq->yawRotationDegrees );
			}
			break;*/
		}
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::RequestUDPConnection()
{
	g_networkingSystem->ConnectTCPClient( m_ipAddress, m_tcpPort );

	RequestConnectionRequest req( m_remoteClientId );
	g_networkingSystem->SendTCPMessage( &req, sizeof( req ) );
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::Update()
{
	if ( g_game != nullptr )
	{
		g_game->UpdateWorldMesh();
		g_game->UpdateEntityAnimations();
	}

	if ( m_playerClient != nullptr )
	{
		m_playerClient->Update();
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ReceiveClientRequests( const std::vector<const ClientRequest*> clientRequests )
{
	for ( int reqIdx = 0; reqIdx < (int)clientRequests.size(); ++reqIdx )
	{
		if ( clientRequests[reqIdx] == nullptr )
		{
			continue;
		}

		const ClientRequest* req = clientRequests[reqIdx];

		switch ( req->functionType )
		{
			case eClientFunctionType::CREATE_ENTITY:
			{
				CreateEntityRequest* createEntityReq = (CreateEntityRequest*)req;
				createEntityReq->clientId = m_remoteClientId;
				g_networkingSystem->SendUDPMessage( m_udpSendToPort, createEntityReq, sizeof( *createEntityReq ), true );

			}
			break;

			case eClientFunctionType::UPDATE_ENTITY:
			{
				UpdateEntityRequest* updateEntityReq = (UpdateEntityRequest*)req;
				updateEntityReq->clientId = m_remoteClientId;
				g_networkingSystem->SendUDPMessage( m_udpSendToPort, updateEntityReq, sizeof( *updateEntityReq ) );
			}
			break;

			case eClientFunctionType::SHOOT:
			{
				ShootRequest* shootReq = (ShootRequest*)req;
				shootReq->clientId = m_remoteClientId;
				g_networkingSystem->SendUDPMessage( m_udpSendToPort, shootReq, sizeof( *shootReq ) );
			}
			break;
		}

	}
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::RegisterNewClient( Client* client )
{
	m_playerClient = (PlayerClient*)client;
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::SendMessageToAllDistantClients( ClientRequest* clientRequest )
{
	UNUSED( clientRequest );
}

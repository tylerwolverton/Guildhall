#include "Game/RemoteClient.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Networking/NetworkingSystem.hpp"
#include "Engine/Time/Time.hpp"
#include "Game/Server.hpp"
#include "Game/Game.hpp"
#include "Game/GameEvents.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
RemoteClient::RemoteClient( ConnectionInfo connectionInfo )
	: Client()
	, m_connectionInfo( connectionInfo )
{

}


//-----------------------------------------------------------------------------------------------
void RemoteClient::Startup()
{
	// Some config to talk to server
	/*RemoteClientRegistrationRequest req( m_clientId );

	g_networkingSystem->SendUDPMessage( 4820, &req, sizeof( req ) );*/

	m_lastUpdateTime = GetCurrentTimeSeconds();
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::Shutdown()
{

}


//-----------------------------------------------------------------------------------------------
void RemoteClient::Update()
{
	ProcessUDPMessages();

	// Retry initial message if not acked yet
	//if ( m_remoteServerInitState == eInitializationState::SENT )
	//{
	//	RemoteClientRegistrationRequest req( m_clientId );
	//	g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, &req, sizeof( req ) );

	//	return;
	//}

	//// Retry set player id message if not acked yet
	//if ( m_remoteServerPlayerIdInitState == eInitializationState::SENT )
	//{
	//	SetPlayerIdRequest req( m_clientId, m_playerId );
	//	g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, &req, sizeof( req ) );

	//	return;
	//}	

	SendUpdatedGameWorldToServer();
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SendUpdatedGameWorldToServer()
{
	// Don't try to send entity updates if the connection isn't up yet
	if ( m_remoteServerInitState != eInitializationState::ACKED
		 || m_remoteServerPlayerIdInitState != eInitializationState::ACKED )
	{
		return;
	}

	// Send initial state if everything is acked and it hasn't been sent yet
	if ( !m_hasSentInitialState )
	{
		std::vector<Entity*> entities = g_game->GetEntitiesInCurrentMap();

		for ( Entity* entity : entities )
		{
			CreateEntityRequest req( m_clientId, entity->GetId(), entity->GetType(), entity->GetPosition(), entity->GetOrientationDegrees() );

			g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, &req, sizeof( req ), true );
			Sleep( 10 );
		}

		m_hasSentInitialState = true;
	}
	else
	{
		if ( GetCurrentTimeSeconds() - m_lastUpdateTime > .1f )
		{
			g_game->UpdatePlayerScoresForAllClients();
		}

		std::vector<Entity*> entities = g_game->GetLivingEntitiesInCurrentMap();

		for ( Entity* entity : entities )
		{
			if ( entity == nullptr )
			{
				continue;
			}

			UpdateEntityOnRemoteServerRequest req( m_clientId, entity->GetId(), entity->GetPosition(), entity->GetOrientationDegrees() );
			//UpdateEntityRequest req( m_clientId, entity->GetId(), entity->GetPosition(), entity->GetOrientationDegrees() );

			g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, &req, sizeof( req ) );

			//std::this_thread::sleep_for( std::chrono::microseconds( 2 ) );
		}

		m_lastUpdateTime = GetCurrentTimeSeconds();

		// Update client's deltaSeconds
		ServerLastDeltaSecondsRequest lastDeltaSecondsReq( m_clientId, g_game->GetLastDeltaSeconds() );
		g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, &lastDeltaSecondsReq, sizeof( lastDeltaSecondsReq ) );
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SendMessageToDistantClient( ClientRequest* message )
{
	switch ( message->functionType )
	{
		case eClientFunctionType::CREATE_ENTITY:
		{
			CreateEntityRequest* createEntityReq = (CreateEntityRequest*)message;
			g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, createEntityReq, sizeof( *createEntityReq ), true );
		}
		break;

		case eClientFunctionType::NOTIFY_ENTITY_DIED:
		{
			NotifyEntityDiedRequest* notifyEntityDiedReq = (NotifyEntityDiedRequest*)message;
			g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, notifyEntityDiedReq, sizeof( *notifyEntityDiedReq ), true );
		}
		break;

		case eClientFunctionType::UPDATE_PLAYER_SCORE:
		{
			UpdatePlayerScoreRequest* updatePlayerScoreReq = (UpdatePlayerScoreRequest*)message;
			g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, updatePlayerScoreReq, sizeof( *updatePlayerScoreReq ) );
		}
		break;

		case eClientFunctionType::DRAW_SHOT:
		{
			DrawShotRequest* drawShotReq = (DrawShotRequest*)message;
			g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, drawShotReq, sizeof( *drawShotReq ), true, 5 );
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SetClientId( int id )
{
	m_clientId = id;
	// Send a message to RemoteServer to set player client's id
	RemoteClientRegistrationRequest req( m_clientId );
	g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, &req, sizeof( req ), true, 100000 );

	m_remoteServerInitState = eInitializationState::SENT;
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SetPlayer( Entity* entity )
{
	if ( m_playerId != -1 )
	{
		return;
	}

	m_playerId = entity->GetId();

	SetPlayerIdRequest req( m_clientId, m_playerId );
	g_networkingSystem->SendUDPMessage( m_connectionInfo.distantSendToPort, &req, sizeof( req ), true, 100000 );

	g_game->AddPlayerScore( m_clientId, m_playerId );

	m_remoteServerInitState = eInitializationState::ACKED;
	m_remoteServerPlayerIdInitState = eInitializationState::SENT;
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::ProcessUDPMessages()
{
	std::vector<const ClientRequest*> gameRequests;

	std::vector<UDPData>& newMessages = g_networkingSystem->ReceiveUDPMessages();

	for ( UDPData& data : newMessages )
	{
		if ( data.GetData() == nullptr 
			 || data.GetFromPort() != m_connectionInfo.localBindPort )
		{
			continue;
		}

		const ClientRequest* req = reinterpret_cast<const ClientRequest*>( data.GetPayload() );
		switch ( req->functionType )
		{
			case eClientFunctionType::CREATE_ENTITY:
			{
				CreateEntityRequest* createEntityReq = (CreateEntityRequest*)req;
				gameRequests.push_back( createEntityReq );
			}
			break;

			case eClientFunctionType::UPDATE_ENTITY:
			{
				// TEMP HACK for prediction like movement
				double roughRoundTripTime = ( GetCurrentTimeSeconds() - m_lastUpdateTime ) * 2.0;

				float multiplier = 200.f;
				if ( roughRoundTripTime < .01 )
				{
					multiplier *= 50.f;
				}

				UpdateEntityRequest* updateEntityReq = (UpdateEntityRequest*)req;
				//updateEntityReq->translationVec *= ( multiplier * (float)roughRoundTripTime );
				updateEntityReq->yawRotationDegrees *= 500.f * g_game->GetLastDeltaSeconds();//( multiplier * (float)roughRoundTripTime );
				gameRequests.push_back( updateEntityReq );
			}
			break;

			case eClientFunctionType::SET_PLAYER_ID_ACK:
			{
				m_remoteServerPlayerIdInitState = eInitializationState::ACKED;
			}
			break;

			case eClientFunctionType::SHOOT:
			{
				ShootRequest* shootReq = (ShootRequest*)req;
				gameRequests.push_back( shootReq );
			}
			break;
		}

		data.Process();
	}

	g_server->ReceiveClientRequests( gameRequests );
}


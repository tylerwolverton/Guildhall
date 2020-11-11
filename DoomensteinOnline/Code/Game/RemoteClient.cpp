#include "Game/RemoteClient.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Networking/NetworkingSystem.hpp"
#include "Game/Server.hpp"
#include "Game/Game.hpp"
#include "Game/GameEvents.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
void RemoteClient::Startup()
{
	// Some config to talk to server
	/*RemoteClientRegistrationRequest req( m_clientId );

	g_networkingSystem->SendUDPMessage( 4820, &req, sizeof( req ) );*/
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
	if ( m_remoteServerInitState == eInitializationState::SENT )
	{
		RemoteClientRegistrationRequest req( m_clientId );
		g_networkingSystem->SendUDPMessage( 4908, &req, sizeof( req ) );
	}

	// Retry set player id message if not acked yet
	if ( m_remoteServerPlayerIdInitState == eInitializationState::SENT )
	{
		SetPlayerIdRequest req( m_clientId, m_playerId );
		g_networkingSystem->SendUDPMessage( 4908, &req, sizeof( req ) );
	}

	// Send initial state if everything is acked and it hasn't been sent yet
	if ( !m_hasSentInitialState )
	{
		if ( m_remoteServerInitState == eInitializationState::ACKED
			 && m_remoteServerPlayerIdInitState == eInitializationState::ACKED )
		{
			std::vector<Entity*> entities = g_game->GetEntitiesInCurrentMap();

			for ( Entity* entity : entities )
			{
				CreateEntityRequest req( m_clientId, entity->GetId(), entity->GetType(), entity->GetPosition(), entity->GetOrientationDegrees() );

				g_networkingSystem->SendUDPMessage( 4908, &req, sizeof( req ) );
			}

			m_hasSentInitialState = true;
		}
	}
	else
	{
		std::vector<Entity*> entities = g_game->GetEntitiesInCurrentMap();

		for ( Entity* entity : entities )
		{
			UpdateEntityOnRemoteServerRequest req( m_clientId, entity->GetId(), entity->GetPosition(), entity->GetOrientationDegrees() );

			g_networkingSystem->SendUDPMessage( 4908, &req, sizeof( req ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SetClientId( int id )
{
	m_clientId = id;
	// Send a message to RemoteServer to set player client's id
	RemoteClientRegistrationRequest req( m_clientId );
	g_networkingSystem->SendUDPMessage( 4908, &req, sizeof( req ) );

	m_remoteServerInitState = eInitializationState::SENT;
}


//-----------------------------------------------------------------------------------------------
void RemoteClient::SetPlayer( Entity* entity )
{
	m_playerId = entity->GetId();

	SetPlayerIdRequest req( m_clientId, m_playerId );
	//SetPlayerIdRequest req( m_clientId, entity->GetId() );
	g_networkingSystem->SendUDPMessage( 4908, &req, sizeof( req ) );

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
		if ( data.GetData() == nullptr )
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
				UpdateEntityRequest* updateEntityReq = (UpdateEntityRequest*)req;
				gameRequests.push_back( updateEntityReq );
			}
			break;

			case eClientFunctionType::SET_PLAYER_ID_ACK:
			{
				m_remoteServerPlayerIdInitState = eInitializationState::ACKED;
			}
			break;
		}

		data.Process();
	}

	g_server->ReceiveClientRequests( gameRequests );
}


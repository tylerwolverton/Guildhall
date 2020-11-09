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


//-----------------------------------------------------------------------------------------------
RemoteServer::RemoteServer( EventArgs* args )
	: Server( args )
{
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::Startup( eAppMode appMode )
{
	RequestUDPConnection();

	StartGame( appMode );
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
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ProcessNetworkMessages()
{
	ProcessTCPMessages();
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

				g_devConsole->PrintString( Stringf( "Response: key = '%i', port '%i', ip = '%s'", responseReq->connectKey, responseReq->port, ipAddress.c_str() ) );

				g_networkingSystem->DisconnectTCPClient();

				KeyVerificationRequest keyVerifyReq( m_remoteClientId, responseReq->connectKey );

				g_networkingSystem->OpenUDPPort( 4820, responseReq->port );
				g_networkingSystem->SendUDPMessage( 4820, &keyVerifyReq, sizeof( keyVerifyReq ) );
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ProcessUDPMessages()
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
			case eClientFunctionType::REMOTE_CLIENT_REGISTRATION:
			{
				m_remoteClientId = req->clientId;
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::RequestUDPConnection()
{
	g_networkingSystem->ConnectTCPClient( m_ipAddress, m_tcpPort );

	RequestConnectionRequest req( m_remoteClientId );
	g_networkingSystem->SendTCPMessage( &req, sizeof( req ) );

	/*m_tcpClientSocket = g_networkingSystem->ConnectTCPClientToServer( m_ipAddress, m_tcpPort );

	if ( m_tcpClientSocket->IsValid() )
	{
		std::array<char, 256> buffer;
		RequestConnectionRequest* req = new RequestConnectionRequest( -1 );
		
		memcpy( &buffer[0], (void*)req, sizeof(*req) );

		m_tcpClientSocket->Send( &buffer[0], sizeof( *req ) );

		delete req;
	}*/
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::Update()
{
	// Copy state of AuthoritativeServer to game
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

		ClientRequest req = *(clientRequests[reqIdx]);
		req.clientId = m_remoteClientId;

		g_networkingSystem->SendUDPMessage( 4820, &req, sizeof( req ) );
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::RegisterNewClient( Client* client )
{
	m_playerClient = client;
}

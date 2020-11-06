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

	PTR_SAFE_DELETE( m_tcpClientSocket );
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::BeginFrame()
{
	// ProcessNetworkMessages();
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
	
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ProcessTCPMessages()
{
	if ( m_tcpClientSocket == nullptr )
	{
		return;
	}

	TCPData data = m_tcpClientSocket->Receive();
	if ( data.GetData() == nullptr )
	{
		return;
	}

	// Process message
	const MessageHeader* header = reinterpret_cast<const MessageHeader*>( data.GetData() );
	switch ( header->id )
	{
		case (uint16_t)eMessasgeProtocolIds::SERVER_LISTENING:
		{
			const char* dataStr = data.GetData() + 4;

			g_devConsole->PrintString( Stringf( "Connected to game: %s", dataStr ) );
		}
		break;

		default:
		{
			g_devConsole->PrintError( Stringf( "Received msg with unknown id: %i", header->id ) );
			return;
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void RemoteServer::ProcessUDPMessages()
{

}


//-----------------------------------------------------------------------------------------------
void RemoteServer::NegotiateUDPConnection()
{
	m_tcpClientSocket = g_networkingSystem->ConnectTCPClientToServer( m_ipAddress, m_tcpPort );
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


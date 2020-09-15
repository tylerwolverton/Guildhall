#include "Engine/Networking/NetworkingSystem.hpp"
#include "Engine/Networking/MessageProtocols.hpp"
#include "Engine/Networking/TCPClient.hpp"
#include "Engine/Networking/TCPServer.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <array>
#include <winsock2.h>
#include <ws2tcpip.h>


//-----------------------------------------------------------------------------------------------
// Winsock Library link
//-----------------------------------------------------------------------------------------------
#pragma comment(lib, "Ws2_32.lib")


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::Startup()
{
	g_eventSystem->RegisterMethodEvent( "start_tcp_server", "Start TCP server listening for client connections, port=48000.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::StartTCPServer );
	g_eventSystem->RegisterMethodEvent( "stop_tcp_server", "Stop TCP server listening for client connections.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::StopTCPServer );
	g_eventSystem->RegisterMethodEvent( "connect", "Connect to TCP server on given host, host=<ip4 address>:<port number>.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::ConnectTCPClient );
	g_eventSystem->RegisterMethodEvent( "disconnect", "Disconnect TCP client from TCP server.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::DisconnectTCPClient );
	g_eventSystem->RegisterMethodEvent( "send_message", "Send a message, msg=\"<message text>\"", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::SendMessage );

	// Initialize winsock
	WSADATA wsaData;
	WORD wVersion MAKEWORD( 2, 2 );
	int iResult = WSAStartup( wVersion, &wsaData );
	if ( iResult != 0 )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSAStartup failed with '%i'", WSAGetLastError() ) );
	}
/*
	FD_ZERO( &m_listenSet );
	m_timeval.tv_sec = 0l;
	m_timeval.tv_usec = 0l;*/

	m_tcpServer = new TCPServer( eBlockingMode::NONBLOCKING );
	m_tcpClient = new TCPClient( eBlockingMode::NONBLOCKING );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::BeginFrame()
{
	// Check if we are a server that's listening
	if ( m_tcpServer->IsListening() )
	{
		if ( !m_serverSocket.IsValid() )
		{
			CheckForNewClientConnection();
		}
		else
		{
			if ( m_serverSocket.IsDataAvailable() )
			{
				ReceiveMessageFromClient();
			}
		}
	}
	// Check if we are a client
	else if ( m_clientSocket.IsValid() )
	{
		if ( m_clientSocket.IsDataAvailable() )
		{
			ReceiveMessageFromServer();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::EndFrame()
{

}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::Shutdown()
{
	PTR_SAFE_DELETE( m_tcpClient );
	PTR_SAFE_DELETE( m_tcpServer );

	int iResult = WSACleanup();
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSACleanup failed with '%i'", WSAGetLastError() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::CheckForNewClientConnection()
{
	m_serverSocket = m_tcpServer->Accept();

	if ( !m_serverSocket.IsValid() )
	{
		return;
	}

	// We found a new connection, send a server listening and print connection info
	g_devConsole->PrintString( Stringf( "Client connected from: %s", m_serverSocket.GetAddress().c_str() ) );

	std::string gameName( "Doomenstein" );
	ServerListeningMsg msg;
	msg.header.id = 1;
	msg.header.size = ( std::uint16_t )gameName.size();

	msg.gameName = gameName;

	m_serverSocket.Send( reinterpret_cast<char*>( &msg ), sizeof( msg ) );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ReceiveMessageFromServer()
{
	TCPData data = m_clientSocket.Receive();
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
			const ServerListeningMsg* svrListeningMsg = reinterpret_cast<const ServerListeningMsg*>( data.GetData() );

			g_devConsole->PrintString( Stringf( "Connected to game: %s", svrListeningMsg->gameName.c_str() ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::TEXT:
		{
			const TextMsg* textMsg = reinterpret_cast<const TextMsg*>( data.GetData() );

			g_devConsole->PrintString( Stringf( "Received from server: %s", textMsg->msg.c_str() ) );
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
void NetworkingSystem::ReceiveMessageFromClient()
{
	TCPData data = m_serverSocket.Receive();
	if ( data.GetData() == nullptr )
	{
		return;
	}

	// Process message
	const MessageHeader* header = reinterpret_cast<const MessageHeader*>( data.GetData() );
	switch ( header->id )
	{
		case (uint16_t)eMessasgeProtocolIds::TEXT:
		{
			const TextMsg* textMsg = reinterpret_cast<const TextMsg*>( data.GetData() );

			g_devConsole->PrintString( Stringf( "Received from client: %s", textMsg->msg.c_str() ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::CLIENT_DISCONNECTING:
		{
			m_clientSocket.Close();
			m_serverSocket.Close();
			g_devConsole->PrintString( Stringf( "Client disconnected" ) );
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
void NetworkingSystem::StartTCPServer( EventArgs* args )
{
	if ( m_tcpServer == nullptr )
	{
		g_devConsole->PrintError( Stringf( "No TCPServer exists to start" ) );
		return;
	}

	int listenPort = args->GetValue( "port", 48000 );
	m_tcpServer->Bind( listenPort );

	m_tcpServer->StartListening();

	g_devConsole->PrintString( Stringf( "Server listening on port '%i'", listenPort ) );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::StopTCPServer( EventArgs* args )
{
	UNUSED( args );

	if ( m_tcpServer == nullptr )
	{
		g_devConsole->PrintError( Stringf( "No TCPServer exists to stop" ) );
		return;
	}

	m_tcpServer->StopListening();

	m_clientSocket.Close();
	m_serverSocket.Close();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ConnectTCPClient( EventArgs* args )
{
	std::string connectionAddress = args->GetValue( "host", ":48000" );

	Strings connectionComponents = SplitStringOnDelimiter( connectionAddress, ':' );

	if ( connectionComponents.size() != 2 )
	{
		g_devConsole->PrintError( Stringf( "Host must be in the form <ip4 address>:<port number>" ) );
		return;
	}

	m_clientSocket = m_tcpClient->Connect( connectionComponents[0], atoi( connectionComponents[1].c_str() ) );

	if ( m_clientSocket.IsValid() )
	{
		std::string hostName( connectionComponents[0] );
		if ( hostName.empty() )
		{
			hostName = "localhost";
		}

		g_devConsole->PrintString( Stringf( "Connected to server '%s:%s'", hostName.c_str(), connectionComponents[1].c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::DisconnectTCPClient( EventArgs* args )
{
	UNUSED( args );

	if ( m_clientSocket.IsValid() )
	{
		ClientDisconnectingMsg msg;
		msg.header.id = (uint16_t)eMessasgeProtocolIds::CLIENT_DISCONNECTING;
		msg.header.size = 0;

		m_clientSocket.Send( reinterpret_cast<char*>( &msg ), sizeof( msg ) );
	}

	m_tcpClient->Disconnect();

	m_clientSocket.Close();
	m_serverSocket.Close();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendMessage( EventArgs* args )
{
	std::string msg = args->GetValue( "msg", "" );

	TextMsg textMsg;
	textMsg.header.id = (uint16_t)eMessasgeProtocolIds::TEXT;
	textMsg.header.size = (uint16_t)msg.size();

	textMsg.msg = msg;

	// Send from server to clients
	if ( m_serverSocket.IsValid() )
	{
		m_serverSocket.Send( reinterpret_cast<char*>( &textMsg ), sizeof( textMsg ) );

		//m_serverSocket.Send( msg.c_str(), msg.size() );
	}

	// Send from client to servers
	else if ( m_clientSocket.IsValid() )
	{
		m_clientSocket.Send( reinterpret_cast<char*>( &textMsg ), sizeof( textMsg ) );
		//m_clientSocket.Send( msg.c_str(), msg.size() );
	}
}

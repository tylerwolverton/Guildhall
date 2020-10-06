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
	// tcp commands
	g_eventSystem->RegisterMethodEvent( "start_tcp_server", "Start TCP server listening for client connections, port=48000.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::StartTCPServer );
	g_eventSystem->RegisterMethodEvent( "stop_tcp_server",	"Stop TCP server listening for client connections.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::StopTCPServer );
	g_eventSystem->RegisterMethodEvent( "connect",			"Connect to TCP server on given host, host=<ip4 address>:<port number>.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::ConnectTCPClient );
	g_eventSystem->RegisterMethodEvent( "disconnect",		"Disconnect TCP client from TCP server.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::DisconnectTCPClient );
	g_eventSystem->RegisterMethodEvent( "send_message",		"Send a message, msg=\"<message text>\"", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::SendMessage );
	
	// udp commands
	g_eventSystem->RegisterMethodEvent( "open_udp_port",	"Open a UDP port and specify target port, bindPort=<port number> sendToPort=<port number>", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::OpenUDPPort );
	g_eventSystem->RegisterMethodEvent( "close_udp_port",	"Close a UDP port, bindPort=<port number>", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::CloseUDPPort );
	g_eventSystem->RegisterMethodEvent( "send_udp_message", "Send a message, msg=\"<message text>\"", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::SendUDPMessage );

	// Initialize winsock
	WSADATA wsaData;
	WORD wVersion MAKEWORD( 2, 2 );
	int iResult = WSAStartup( wVersion, &wsaData );
	if ( iResult != 0 )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSAStartup failed with '%i'", WSAGetLastError() ) );
	}
	
	m_tcpServer = new TCPServer( eBlockingMode::NONBLOCKING );
	m_tcpClient = new TCPClient( eBlockingMode::NONBLOCKING );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::BeginFrame()
{
	ProcessTCPCommunication();

	/*for ( auto& udpSocket : m_udpSockets )
	{
		UDPData data = udpSocket.second->Receive();
		if ( data.GetLength() > 0 )
		{
			g_devConsole->PrintString( Stringf( "Received data from '%s'", data.GetFromAddress().c_str() ) );
		}
	}*/
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::EndFrame()
{

}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::Shutdown()
{
	DisconnectTCPClient( nullptr );
	DisconnectTCPServer( nullptr );

	PTR_SAFE_DELETE( m_tcpClient );
	PTR_SAFE_DELETE( m_tcpServer );

	for ( auto& udpSocket : m_udpSockets )
	{
		udpSocket.second->Close();
	}

	PTR_MAP_SAFE_DELETE( m_udpSockets );

	int iResult = WSACleanup();
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSACleanup failed with '%i'", WSAGetLastError() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ProcessTCPCommunication()
{
	// Check if we are a server that's listening
	if ( m_tcpServer->IsListening() )
	{
		if ( !m_serverSocket.IsValid() )
		{
			CheckForNewTCPClientConnection();
		}
		else
		{
			if ( m_serverSocket.IsDataAvailable() )
			{
				ReceiveMessageFromTCPClient();
			}
		}
	}
	// Check if we are a client
	else if ( m_clientSocket.IsValid() )
	{
		if ( m_clientSocket.IsDataAvailable() )
		{
			ReceiveMessageFromTCPServer();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::CheckForNewTCPClientConnection()
{
	m_serverSocket = m_tcpServer->Accept();

	if ( !m_serverSocket.IsValid() )
	{
		return;
	}

	// We found a new connection, send a server listening and print connection info
	g_devConsole->PrintString( Stringf( "Client connected from: %s", m_serverSocket.GetAddress().c_str() ) );

	std::string gameName( "Doomenstein" );
	std::array<char, 256> buffer;
	MessageHeader* msg = reinterpret_cast<MessageHeader*>( &buffer[0] );
	msg->id = (uint16_t)eMessasgeProtocolIds::SERVER_LISTENING;
	msg->size = ( uint16_t )gameName.size();

	memcpy( &buffer[4], gameName.c_str(), msg->size );

	m_serverSocket.Send( &buffer[0], gameName.size() + 4 );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ReceiveMessageFromTCPServer()
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
			const char* dataStr = data.GetData() + 4;

			g_devConsole->PrintString( Stringf( "Connected to game: %s", dataStr ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::SERVER_DISCONNECTING:
		{
			m_clientSocket.Close();
			m_serverSocket.Close();
			g_devConsole->PrintString( Stringf( "Server disconnected" ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::TEXT:
		{
			const char* dataStr = data.GetData() + 4;

			g_devConsole->PrintString( Stringf( "Received from server: %s", dataStr ) );
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
void NetworkingSystem::ReceiveMessageFromTCPClient()
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
			const char* dataStr = data.GetData() + 4;
			g_devConsole->PrintString( Stringf( "Received from client: %s", dataStr ) );
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

	DisconnectTCPServer( nullptr );

	g_devConsole->PrintString( Stringf( "Server stopped" ) );
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

		m_clientSocket.Send( reinterpret_cast<char*>( &msg ), msg.GetSize() );
	}

	m_tcpClient->Disconnect();

	m_clientSocket.Close();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::DisconnectTCPServer( EventArgs* args )
{
	UNUSED( args );

	if ( m_serverSocket.IsValid() )
	{
		ServerDisconnectingMsg msg;
		msg.header.id = (uint16_t)eMessasgeProtocolIds::SERVER_DISCONNECTING;
		msg.header.size = 0;

		m_serverSocket.Send( reinterpret_cast<char*>( &msg ), msg.GetSize() );
	}
	
	m_serverSocket.Close();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendMessage( EventArgs* args )
{
	std::string msg = args->GetValue( "msg", "" );
	
	std::array<char, 256> buffer;
	MessageHeader* msgHeader = reinterpret_cast<MessageHeader*>( &buffer[0] );

	msgHeader->id = (uint16_t)eMessasgeProtocolIds::TEXT;
	msgHeader->size = (uint16_t)msg.size();

	memcpy( &buffer[4], msg.c_str(), msgHeader->size );

	// Send from server to clients
	if ( m_serverSocket.IsValid() )
	{
		m_serverSocket.Send( &buffer[0], msgHeader->size + 4 );
	}

	// Send from client to servers
	else if ( m_clientSocket.IsValid() )
	{
		m_clientSocket.Send( &buffer[0], msgHeader->size + 4 );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::OpenUDPPort( EventArgs* args )
{
	int bindPort = args->GetValue( "bindPort", 48000 );
	int sendToPort = args->GetValue( "sendToPort", 48001 );
	
	UDPSocket* udpSocket = new UDPSocket( "", sendToPort );

	m_udpSockets[bindPort] = udpSocket;
	udpSocket->Bind( bindPort );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::CloseUDPPort( EventArgs* args )
{
	int bindPort = args->GetValue( "bindPort", 48000 );

	auto iter = m_udpSockets.find( bindPort );
	if ( iter != m_udpSockets.end() )
	{
		iter->second->Close();
		m_udpSockets.erase( bindPort );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendUDPMessage( EventArgs* args )
{
	std::string msg = args->GetValue( "msg", "" );

	std::array<char, 256> buffer;
	MessageHeader* msgHeader = reinterpret_cast<MessageHeader*>( &buffer[0] );

	msgHeader->id = (uint16_t)eMessasgeProtocolIds::TEXT;
	msgHeader->size = (uint16_t)msg.size();

	memcpy( &buffer[4], msg.c_str(), msgHeader->size );

	for ( const auto& udpSocket : m_udpSockets )
	{
		udpSocket.second->Send( &buffer[0], msgHeader->size + 4 );
	}
}

#include "Engine/Networking/NetworkingSystem.hpp"
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
	m_tcpClient = new TCPClient();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::BeginFrame()
{

	if ( m_tcpServer->IsListening() )
	{
		if ( !m_serverSocket.IsValid() )
		{
			m_serverSocket = m_tcpServer->Accept();

			if ( m_serverSocket.IsValid() )
			{
				g_devConsole->PrintString( Stringf( "Client connected from: %s", m_serverSocket.GetAddress().c_str() ) );
			}
		}
		else
		{

		}
	}
	/*if ( m_serverSocket.IsValid() )
	{

	}*/


	//if ( m_isListening )
	//{
	//	// Acquire socket if we don't have one
	//	if ( m_listenSocket == INVALID_SOCKET )
	//	{
	//		addrinfo addrHintsIn;
	//		memset( &addrHintsIn, 0, sizeof( addrHintsIn ) );

	//		addrHintsIn.ai_family = AF_INET;
	//		addrHintsIn.ai_socktype = SOCK_STREAM;
	//		addrHintsIn.ai_protocol = IPPROTO_TCP;
	//		addrHintsIn.ai_flags = AI_PASSIVE;

	//		std::string serverPort( "48000" );
	//		addrinfo* addrInfoOut;

	//		int iResult = getaddrinfo( NULL, serverPort.c_str(), &addrHintsIn, &addrInfoOut );
	//		if ( iResult != 0 )
	//		{
	//			g_devConsole->PrintError( Stringf( "Networking System: getaddrinfo failed with '%i'", iResult ) );
	//			return;
	//		}

	//		m_listenSocket = socket( addrInfoOut->ai_family, addrInfoOut->ai_socktype, addrInfoOut->ai_protocol );
	//		if ( m_listenSocket == INVALID_SOCKET )
	//		{
	//			g_devConsole->PrintError( Stringf( "Networking System: socket creation failed with '%i'", WSAGetLastError() ) );
	//			return;
	//		}

	//		unsigned long blockingMode = 1;
	//		iResult = ioctlsocket( m_listenSocket, FIONBIO, &blockingMode );
	//		if ( iResult == SOCKET_ERROR )
	//		{
	//			g_devConsole->PrintError( Stringf( "Networking System: ioctlsocket failed with '%i'", WSAGetLastError() ) );
	//			return;
	//		}

	//		iResult = bind( m_listenSocket, addrInfoOut->ai_addr, (int)addrInfoOut->ai_addrlen );
	//		if ( iResult == SOCKET_ERROR )
	//		{
	//			g_devConsole->PrintError( Stringf( "Networking System: bind failed with '%i'", WSAGetLastError() ) );
	//			return;
	//		}

	//		iResult = listen( m_listenSocket, SOMAXCONN );
	//		if ( iResult == SOCKET_ERROR )
	//		{
	//			g_devConsole->PrintError( Stringf( "Networking System: listen failed with '%i'", WSAGetLastError() ) );
	//			return;
	//		}
	//	}
	//	// We have a valid socket
	//	else
	//	{
	//		FD_ZERO( &m_listenSet );
	//		FD_SET( m_listenSocket, &m_listenSet );
	//		int iResult = select( 0, &m_listenSet, nullptr, nullptr, &m_timeval );

	//		if ( iResult == INVALID_SOCKET )
	//		{
	//			g_devConsole->PrintError( Stringf( "Networking System: select failed with '%i'", WSAGetLastError() ) );
	//			return;
	//		}

	//		if ( FD_ISSET( m_listenSocket, &m_listenSet ) )
	//		{
	//			m_clientSocket = accept( m_listenSocket, nullptr, nullptr );
	//			if ( m_clientSocket == INVALID_SOCKET )
	//			{
	//				g_devConsole->PrintError( Stringf( "Networking System: client socket accept failed with '%i'", WSAGetLastError() ) );
	//				return;
	//			}
	//			
	//			g_devConsole->PrintString( Stringf( "Client connected from: %s", GetAddress().c_str() ) );
	//		
	//			// Read in data from client
	//			std::array<char, 256> dataBuffer;
	//			iResult = recv( m_clientSocket, &dataBuffer[0], (int)dataBuffer.size() - 1, 0 );
	//			if ( iResult == SOCKET_ERROR )
	//			{
	//				g_devConsole->PrintError( Stringf( "Networking System: recv failed with '%i'", WSAGetLastError() ) );
	//				return;
	//			}
	//			else if ( iResult == 0 )
	//			{
	//				g_devConsole->PrintError( Stringf( "Networking System: recv returned 0 bytes" ) );
	//				return;
	//			}
	//			
	//			// Terminate the buffer string
	//			dataBuffer[iResult] = '\0';
	//			g_devConsole->PrintString( Stringf( "Client message: %s", &dataBuffer[0] ) );

	//			std::string msg( "Hey client" );
	//			iResult = send( m_clientSocket, msg.data(), (int)msg.size(), 0 );
	//			if ( iResult == SOCKET_ERROR )
	//			{
	//				g_devConsole->PrintError( Stringf( "Networking System: send failed with '%i'", WSAGetLastError() ) );
	//				return;
	//			}
	//		}
	//	}
	//}
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
//std::string NetworkingSystem::GetAddress()
//{
//	std::array<char, 128> addressStr;
//
//	sockaddr clientAddr;
//	int addrSize = sizeof( clientAddr );
//	int iResult = getpeername( m_clientSocket, &clientAddr, &addrSize );
//	if ( iResult == SOCKET_ERROR )
//	{
//		g_devConsole->PrintError( Stringf( "Networking System: getpeername failed with '%i'", WSAGetLastError() ) );
//	}
//
//	DWORD outlen = (DWORD)addressStr.size();
//	iResult = WSAAddressToStringA( &clientAddr, addrSize, NULL, &addressStr[0], &outlen );
//	if ( iResult == SOCKET_ERROR )
//	{
//		g_devConsole->PrintError( Stringf( "Networking System: WSAAddressToStringA failed with '%i'", WSAGetLastError() ) );
//		return "";
//	}
//
//	// Is this safe?
//	addressStr[outlen - 1] = '\0';
//	return std::string( &addressStr[0] );
//}


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

	m_clientSocket.Close();
}



#include "Engine/Networking/NetworkingSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

//-----------------------------------------------------------------------------------------------
// Winsock Library link
//-----------------------------------------------------------------------------------------------
#pragma comment(lib, "Ws2_32.lib")


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::Startup()
{
	//g_eventSystem->RegisterEvent( "start_tcp_server", "Start TCP server listening for client connections, port=48000.", eUsageLocation::DEV_CONSOLE, StartTCPServer );
	g_eventSystem->RegisterMethodEvent( "start_tcp_server", "Start TCP server listening for client connections, port=48000.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::StartTCPServer );

	// Initialize winsock
	WSADATA wsaData;
	WORD wVersion MAKEWORD( 2, 2 );
	int iResult = WSAStartup( wVersion, &wsaData );
	if ( iResult != 0 )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSAStartup failed with '%i'", WSAGetLastError() ) );
	}

	m_listenSocket = INVALID_SOCKET;
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::BeginFrame()
{
	if ( m_isListening )
	{
		// Acquire socket if we don't have one
		if ( m_listenSocket == INVALID_SOCKET )
		{
			addrinfo addrHintsIn;
			memset( &addrHintsIn, 0, sizeof( addrHintsIn ) );

			addrHintsIn.ai_family = AF_INET;
			addrHintsIn.ai_socktype = SOCK_STREAM;
			addrHintsIn.ai_protocol = IPPROTO_TCP;
			addrHintsIn.ai_flags = AI_PASSIVE;

			std::string serverPort( "48000" );
			addrinfo* addrInfoOut;

			int iResult = getaddrinfo( NULL, serverPort.c_str(), &addrHintsIn, &addrInfoOut );
			if ( iResult != 0 )
			{
				g_devConsole->PrintError( Stringf( "Networking System: getaddrinfo failed with '%i'", iResult ) );
				return;
			}

			m_listenSocket = socket( addrInfoOut->ai_family, addrInfoOut->ai_socktype, addrInfoOut->ai_protocol );
			if ( m_listenSocket == INVALID_SOCKET )
			{
				g_devConsole->PrintError( Stringf( "Networking System: socket creation failed with '%i'", WSAGetLastError() ) );
				return;
			}

			unsigned long blockingMode = 1;
			iResult = ioctlsocket( m_listenSocket, FIONBIO, &blockingMode );
			if ( iResult == SOCKET_ERROR )
			{
				g_devConsole->PrintError( Stringf( "Networking System: ioctlsocket failed with '%i'", WSAGetLastError() ) );
				return;
			}

			iResult = bind( m_listenSocket, addrInfoOut->ai_addr, (int)addrInfoOut->ai_addrlen );
			if ( iResult == SOCKET_ERROR )
			{
				g_devConsole->PrintError( Stringf( "Networking System: bind failed with '%i'", WSAGetLastError() ) );
				return;
			}

			iResult = listen( m_listenSocket, SOMAXCONN );
			if ( iResult == SOCKET_ERROR )
			{
				g_devConsole->PrintError( Stringf( "Networking System: listen failed with '%i'", WSAGetLastError() ) );
				return;
			}
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
	int iResult = WSACleanup();
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSACleanup failed with '%i'", WSAGetLastError() ) );
	}
}

//-----------------------------------------------------------------------------------------------
void NetworkingSystem::StartTCPServer( EventArgs* args )
{
	m_isListening = true;
	m_listenPort = args->GetValue( "port", 48000 );
}

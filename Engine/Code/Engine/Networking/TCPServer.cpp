#include "Engine/Networking/TCPServer.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
TCPServer::TCPServer()
{
	FD_ZERO( &m_listenSet );
}


//-----------------------------------------------------------------------------------------------
bool TCPServer::Bind( int port )
{
	addrinfo addrHintsIn;
	memset( &addrHintsIn, 0, sizeof( addrHintsIn ) );

	addrHintsIn.ai_family = AF_INET;
	addrHintsIn.ai_socktype = SOCK_STREAM;
	addrHintsIn.ai_protocol = IPPROTO_TCP;
	addrHintsIn.ai_flags = AI_PASSIVE;

	addrinfo* addrInfoOut;
	std::string serverPort = Stringf( "%i", port );
	int iResult = getaddrinfo( NULL, serverPort.c_str(), &addrHintsIn, &addrInfoOut );
	if ( iResult != 0 )
	{
		g_devConsole->PrintError( Stringf( "Networking System: getaddrinfo failed with '%i'", iResult ) );
		freeaddrinfo( addrInfoOut );
		return false;
	}

	m_listenSocket = socket( addrInfoOut->ai_family, addrInfoOut->ai_socktype, addrInfoOut->ai_protocol );
	if ( m_listenSocket == INVALID_SOCKET )
	{
		g_devConsole->PrintError( Stringf( "Networking System: socket creation failed with '%i'", WSAGetLastError() ) );
		freeaddrinfo( addrInfoOut );
		return false;
	}

	unsigned long blockingMode = 1;
	iResult = ioctlsocket( m_listenSocket, FIONBIO, &blockingMode );
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: ioctlsocket failed with '%i'", WSAGetLastError() ) );
		freeaddrinfo( addrInfoOut );
		return false;
	}

	iResult = bind( m_listenSocket, addrInfoOut->ai_addr, (int)addrInfoOut->ai_addrlen );
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: bind failed with '%i'", WSAGetLastError() ) );
		freeaddrinfo( addrInfoOut );
		return false;
	}

	freeaddrinfo( addrInfoOut );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool TCPServer::StartListening()
{
	int iResult = listen( m_listenSocket, SOMAXCONN );
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: listen failed with '%i'", WSAGetLastError() ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool TCPServer::StopListening()
{
	if ( m_listenSocket != INVALID_SOCKET )
	{
		int iResult = closesocket( m_listenSocket );
		if ( iResult == SOCKET_ERROR )
		{
			g_devConsole->PrintError( Stringf( "Networking System: closesocket failed with '%i'", WSAGetLastError() ) );
			return false;
		}
		m_listenSocket = INVALID_SOCKET;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
SOCKET TCPServer::Accept()
{
	SOCKET socket = INVALID_SOCKET;
	if ( m_blockingMode == eBlockingMode::NONBLOCKING )
	{
		FD_ZERO( &m_listenSet );
		FD_SET( m_listenSocket, &m_listenSet );
		int iResult = select( 0, &m_listenSet, NULL, NULL, &m_timeval );
		if ( iResult == SOCKET_ERROR )
		{
			g_devConsole->PrintError( Stringf( "Networking System: select failed with '%i'", WSAGetLastError() ) );
			
			closesocket( m_listenSocket );
			return INVALID_SOCKET;
		}
	}
	
	if ( m_blockingMode == eBlockingMode::BLOCKING
		 || ( m_blockingMode == eBlockingMode::NONBLOCKING && FD_ISSET( m_listenSocket, &m_listenSet ) ) )
	{
		// Accept client connections
		socket = accept( m_listenSocket, NULL, NULL );
		if ( socket == INVALID_SOCKET )
		{
			g_devConsole->PrintError( Stringf( "Networking System: client socket accept failed with '%i'", WSAGetLastError() ) );
			closesocket( m_listenSocket );
			return INVALID_SOCKET;
		}
	}

	return socket;
}

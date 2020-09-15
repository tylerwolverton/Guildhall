#include "Engine/Networking/TCPClient.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Networking/TCPSocket.hpp"

#include <cstdint>


//-----------------------------------------------------------------------------------------------
TCPClient::TCPClient( eBlockingMode mode )
	: m_blockingMode( mode )
{

}


//-----------------------------------------------------------------------------------------------
TCPSocket TCPClient::Connect( const std::string& host, int port )
{
	// Resolve the port locally
	struct addrinfo  addrHintsIn;
	struct addrinfo* addrInfoOut = NULL;

	ZeroMemory( &addrHintsIn, sizeof( addrHintsIn ) );
	addrHintsIn.ai_family = AF_INET;
	addrHintsIn.ai_socktype = SOCK_STREAM;
	addrHintsIn.ai_protocol = IPPROTO_TCP;

	// Get the Server IP address and port
	std::string serverPort = Stringf( "%i", port );
	int iResult = getaddrinfo( host.c_str(), serverPort.c_str(), &addrHintsIn, &addrInfoOut );
	if ( iResult != 0 )
	{
		g_devConsole->PrintError( Stringf( "Networking System: getaddrinfo failed with '%i'", iResult ) );
	
		return TCPSocket( INVALID_SOCKET );
	}

	// Create the connection socket.
	m_socket = socket( addrInfoOut->ai_family, addrInfoOut->ai_socktype, addrInfoOut->ai_protocol );
	if ( m_socket == INVALID_SOCKET )
	{
		g_devConsole->PrintError( Stringf( "Networking System: socket creation failed with '%i'", WSAGetLastError() ) );
		freeaddrinfo( addrInfoOut );
		return TCPSocket( INVALID_SOCKET );
	}

	// Connect to the server
	iResult = connect( m_socket, addrInfoOut->ai_addr, (int)addrInfoOut->ai_addrlen );
	if ( iResult == SOCKET_ERROR )
	{
		closesocket( m_socket );
		m_socket = INVALID_SOCKET;
	}
	freeaddrinfo( addrInfoOut );

	if ( m_socket == INVALID_SOCKET )
	{
		g_devConsole->PrintError( Stringf( "Networking System: connection failed" ) );
		return TCPSocket( INVALID_SOCKET );
	}

	// Set blocking mode as needed.
	if ( m_blockingMode == eBlockingMode::NONBLOCKING )
	{
		u_long winsockmode = 1;
		iResult = ioctlsocket( m_socket, FIONBIO, &winsockmode );
		if ( iResult == SOCKET_ERROR )
		{
			g_devConsole->PrintError( Stringf( "Networking System: ioctlsocket failed with '%i'", WSAGetLastError() ) );
			closesocket( m_socket );
			return TCPSocket( INVALID_SOCKET );
		}
	}

	return TCPSocket( m_socket, m_blockingMode );
}


//-----------------------------------------------------------------------------------------------
void TCPClient::Disconnect()
{
	closesocket( m_socket );
	m_socket = INVALID_SOCKET;
}

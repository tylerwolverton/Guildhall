#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Core/DevConsole.hpp"

#include <iostream>


//-----------------------------------------------------------------------------------------------
UDPSocket::UDPSocket( const std::string& host, int port )
{
	m_toAddress.sin_family = AF_INET;
	m_toAddress.sin_port = htons((u_short)port);
	m_toAddress.sin_addr.s_addr = inet_addr( host.c_str() );

	m_socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( m_socket == INVALID_SOCKET )
	{
		LOG_ERROR( "Socket instantiate failed '%i'", WSAGetLastError() );
	}
}


//-----------------------------------------------------------------------------------------------
UDPSocket::~UDPSocket()
{
	Close();
}


//-----------------------------------------------------------------------------------------------
void UDPSocket::Bind( int port )
{
	m_bindAddress.sin_family = AF_INET;
	m_bindAddress.sin_port = htons( (u_short)port );
	m_bindAddress.sin_addr.s_addr = htonl( INADDR_ANY );

	int result = bind( m_socket, (SOCKADDR*)&m_bindAddress, sizeof( m_bindAddress ) );
	if ( result != 0 )
	{
		LOG_ERROR( "Bind failed with '%i'", WSAGetLastError() );
	}
}


//-----------------------------------------------------------------------------------------------
void UDPSocket::Close()
{
	if ( m_socket != INVALID_SOCKET )
	{
		int result = closesocket( m_socket );
		if ( result == SOCKET_ERROR )
		{
			LOG_ERROR( "Socket instantiate failed with '%i'", WSAGetLastError() );
		}

		m_socket = INVALID_SOCKET;
	}
}


//-----------------------------------------------------------------------------------------------
int UDPSocket::Send()
{
	return 0;
}


//-----------------------------------------------------------------------------------------------
int UDPSocket::Receive()
{
	return 0;
}

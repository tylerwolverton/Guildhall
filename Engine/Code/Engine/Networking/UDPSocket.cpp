#include "Engine/Networking/UDPSocket.hpp"

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
		std::cout <<"Socket instantiate failed, error: " << WSAGetLastError() << std::endl;
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
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons( (u_short)port );
	bindAddr.sin_addr.s_addr = htonl( INADDR_ANY );

	int result = bind( m_socket, (SOCKADDR*)&bindAddr, sizeof( bindAddr ) );
	if ( result != 0 )
	{
		std::cout <<"Bind failed with " << WSAGetLastError() << std::endl;
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
			std::cout <<"Socket instantiate failed with " << WSAGetLastError() << std::endl;
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

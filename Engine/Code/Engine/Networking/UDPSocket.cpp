#include "Engine/Networking/UDPSocket.hpp"
#include "Engine/Core/DevConsole.hpp"

#include <iostream>


//-----------------------------------------------------------------------------------------------
UDPSocket::UDPSocket( const std::string& host, int port )
{
	std::string hostAddr( host );
	if ( hostAddr.empty() )
	{
		hostAddr = "127.0.0.1";
	}

	m_toAddress.sin_family = AF_INET;
	m_toAddress.sin_port = htons((u_short)port);
	m_toAddress.sin_addr.s_addr = inet_addr( hostAddr.c_str() );

	m_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( m_socket == INVALID_SOCKET )
	{
		LOG_ERROR( "Socket instantiate failed '%i'", WSAGetLastError() );
	}
}


//-----------------------------------------------------------------------------------------------
UDPSocket::UDPSocket()
	: UDPSocket( "", 48000 )
{

}


//-----------------------------------------------------------------------------------------------
UDPSocket::~UDPSocket()
{
	Close();
}


//-----------------------------------------------------------------------------------------------
void UDPSocket::Bind( int port )
{
	m_bindPort = port;

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
//int UDPSocket::Send( const char* data, size_t length )
int UDPSocket::Send( size_t length )
{
	int bytesSent = sendto( m_socket, &m_sendBuffer[0], (int)length, 0, reinterpret_cast<SOCKADDR*>( &m_toAddress ), sizeof( m_toAddress ) );
	if ( bytesSent == SOCKET_ERROR )
	{
		LOG_ERROR( "Send to failed with '%i'", WSAGetLastError() );
	}
	else if ( bytesSent < (int)length )
	{
		LOG_ERROR( "Requested '%i' bytes to be sent, but only '%i' were sent", (int)length, bytesSent );
		Close();
	}
	
	// Clear sendBuffer?

	return bytesSent;
}


//-----------------------------------------------------------------------------------------------
UDPData UDPSocket::Receive()
{
	sockaddr_in fromAddress;
	int fromAddrLength = sizeof( fromAddress );

	int iResult = recvfrom( m_socket, &m_receiveBuffer[0], (int)m_receiveBuffer.size(), 0, reinterpret_cast<SOCKADDR*>( &fromAddress ), &fromAddrLength );
	if ( iResult == SOCKET_ERROR )
	{
		//LOG_ERROR( "Receive from failed with '%i'", WSAGetLastError() );
		return UDPData();
	}

	if ( iResult > BUFFER_SIZE - 1 )
	{
		LOG_ERROR( "Receive from received too much data for buffer" );
		return UDPData();
	}
	else
	{
		m_receiveBuffer[iResult] = '\0';
	}

	std::string fromAddressStr = std::string( inet_ntoa( fromAddress.sin_addr ) );
	
	return UDPData( iResult, &m_receiveBuffer[6], fromAddressStr );
}


//-----------------------------------------------------------------------------------------------
std::string UDPData::GetFromIPAddress() const
{
	Strings connectionInfo = SplitStringOnDelimiter( m_fromAddress, ':' );

	if ( connectionInfo.size() != 2 )
	{
		return "";
	}

	return connectionInfo[0];
}


//-----------------------------------------------------------------------------------------------
int UDPData::GetFromPort() const
{
	Strings connectionInfo = SplitStringOnDelimiter( m_fromAddress, ':' );

	if ( connectionInfo.size() != 2 )
	{
		return -1;
	}

	return atoi( connectionInfo[1].c_str() );
}

#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <array>


//-----------------------------------------------------------------------------------------------
TCPSocket::TCPSocket( SOCKET socket, eBlockingMode mode, size_t bufferSize )
	: m_socket( socket )
	, m_blockingMode( mode )
	, m_bufferSize( bufferSize )
{
	FD_ZERO( &m_fdSet );
	m_buffer = new char[m_bufferSize];
}


//-----------------------------------------------------------------------------------------------
TCPSocket::TCPSocket()
	: m_socket( INVALID_SOCKET )
	, m_blockingMode( eBlockingMode::BLOCKING )
	, m_bufferSize( 256 )
{
	FD_ZERO( &m_fdSet );
	m_buffer = new char[m_bufferSize];
}


//-----------------------------------------------------------------------------------------------
TCPSocket::~TCPSocket()
{
	PTR_SAFE_DELETE( m_buffer );
}


//-----------------------------------------------------------------------------------------------
TCPSocket& TCPSocket::operator=( const TCPSocket& src )
{
	if ( this != &src )
	{
		m_blockingMode = src.m_blockingMode;
		m_bufferSize = src.m_bufferSize;
		m_receiveSize = src.m_receiveSize;
		m_socket = src.m_socket;
		if ( m_bufferSize > 0 )
		{
			m_buffer = new char[m_bufferSize];
		}
	}
	return *this;
}


//-----------------------------------------------------------------------------------------------
std::string TCPSocket::GetAddress()
{
	std::array<char, 128> addressStr;

	sockaddr clientAddr;
	int addrSize = sizeof( clientAddr );
	int iResult = getpeername( m_socket, &clientAddr, &addrSize );
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: getpeername failed with '%i'", WSAGetLastError() ) );
	}

	DWORD outlen = (DWORD)addressStr.size();
	iResult = WSAAddressToStringA( &clientAddr, addrSize, NULL, &addressStr[0], &outlen );
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSAAddressToStringA failed with '%i'", WSAGetLastError() ) );
		return "";
	}

	// Is this safe?
	addressStr[outlen - 1] = '\0';
	return std::string( &addressStr[0] );
}


//-----------------------------------------------------------------------------------------------
void TCPSocket::Send( const char* data, size_t length )
{
	int iResult = send( m_socket, data, (int)length, 0 );
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: send failed with '%i'", WSAGetLastError() ) );
		closesocket( m_socket );
		return;
	}
	else if ( iResult < (int)length )
	{
		g_devConsole->PrintError( Stringf( "Requested '%i' bytes to be sent, but only '%i' were sent", (int)length, iResult ) );
		closesocket( m_socket );
		return;
	}
}


//-----------------------------------------------------------------------------------------------
TCPData TCPSocket::Receive()
{
	int iResult = recv( m_socket, m_buffer, (int)m_bufferSize, 0 );
	if ( iResult == SOCKET_ERROR )
	{
		int errorCode = WSAGetLastError();
		if ( errorCode == WSAEWOULDBLOCK && m_blockingMode == eBlockingMode::NONBLOCKING )
		{
			return TCPData( 9999999, nullptr );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "Networking System: recv failed with '%i'", errorCode ) );
			closesocket( m_socket );
			return TCPData( 9999999, nullptr );
		}
	}

	return TCPData( size_t( iResult ), m_buffer );
}


//-----------------------------------------------------------------------------------------------
void TCPSocket::Close()
{
	closesocket( m_socket );
	m_socket = INVALID_SOCKET;
}


//-----------------------------------------------------------------------------------------------
bool TCPSocket::IsDataAvailable()
{
	return false;
}

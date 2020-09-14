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
	m_bufferPtr = new char[m_bufferSize];
}


//-----------------------------------------------------------------------------------------------
TCPSocket::~TCPSocket()
{
	delete[] m_bufferPtr;
	m_bufferPtr = nullptr;
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
			m_bufferPtr = new char[m_bufferSize];
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

}


//-----------------------------------------------------------------------------------------------
bool TCPSocket::IsDataAvailable()
{
	return false;
}






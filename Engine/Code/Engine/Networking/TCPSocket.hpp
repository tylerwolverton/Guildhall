#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"

#include <string>


class TCPSocket
{
public:
	TCPSocket( SOCKET socket, eBlockingMode mode = eBlockingMode::BLOCKING, size_t bufferSize = 256 );
	~TCPSocket();

	TCPSocket& operator=( const TCPSocket& src );

	bool IsValid()											{ return m_socket != INVALID_SOCKET; }

	std::string GetAddress();

	void Send( const char* data, size_t length );
	//TCPData Receive();

	bool IsDataAvailable();

private:
	eBlockingMode m_blockingMode = eBlockingMode::INVALID;
	SOCKET m_socket = INVALID_SOCKET;

	FD_SET m_fdSet;
	timeval m_timeval = {0l,0l};

	size_t m_bufferSize = 0;
	size_t m_receiveSize = 0;
	char* m_bufferPtr = nullptr;
};

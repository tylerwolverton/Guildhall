#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"
#include "Engine/Networking/TCPSocket.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class TCPClient
{
	friend class NetworkingSystem;

private:
	TCPClient( eBlockingMode mode = eBlockingMode::BLOCKING );

public:
	~TCPClient() = default;

	TCPSocket Connect( const std::string& host, int port );
	TCPSocket* ConnectAndCreateSocket( const std::string& host, int port );
	void Disconnect();

private:
	eBlockingMode m_blockingMode = eBlockingMode::INVALID;
	SOCKET m_socket;
};

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

	TCPSocket Connect( const std::string& host, int port, eBlockingMode mode = eBlockingMode::BLOCKING );
	void Disconnect();

private:
	eBlockingMode m_blockingMode = eBlockingMode::INVALID;
	SOCKET m_socket;
};

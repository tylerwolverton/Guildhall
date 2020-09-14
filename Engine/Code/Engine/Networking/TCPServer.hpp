#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"


class TCPServer
{
private:
	TCPServer();

public:
	~TCPServer() = default;

	bool Bind( int port );
	bool StartListening();
	bool StopListening();
	SOCKET Accept();

private:
	eBlockingMode m_blockingMode = eBlockingMode::INVALID;
	FD_SET m_listenSet;
	timeval m_timeval = {0l,0l};
	SOCKET m_listenSocket = INVALID_SOCKET;

	bool m_isListening = false;
	int m_listenPort = -1;
};

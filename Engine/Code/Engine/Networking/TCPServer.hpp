#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"


//-----------------------------------------------------------------------------------------------
class TCPSocket;


//-----------------------------------------------------------------------------------------------
class TCPServer
{
	friend class NetworkingSystem;

private:
	TCPServer( eBlockingMode mode = eBlockingMode::BLOCKING );

public:
	~TCPServer() = default;

	bool IsListening() const								{ return m_isListening; }

	bool Bind( int port );
	bool StartListening();
	bool StopListening();
	TCPSocket Accept();

private:
	eBlockingMode m_blockingMode = eBlockingMode::INVALID;
	FD_SET m_listenSet;
	timeval m_timeval = {0l,0l};
	SOCKET m_listenSocket = INVALID_SOCKET;

	bool m_isListening = false;
	int m_listenPort = -1;
};

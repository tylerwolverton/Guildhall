#pragma once
#include "Engine/Core/EngineCommon.hpp"

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>


//-----------------------------------------------------------------------------------------------
class NetworkingSystem
{
public:
	NetworkingSystem() = default;
	~NetworkingSystem() = default;

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	std::string GetAddress();

private:
	void StartTCPServer( EventArgs* args );

private:
	bool m_isListening = false;
	FD_SET m_listenSet;
	timeval m_timeval;
	int m_listenPort = -1;
	SOCKET m_listenSocket = INVALID_SOCKET;
	SOCKET m_clientSocket = INVALID_SOCKET;
};

#pragma once
#include "Engine/Core/EngineCommon.hpp"


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

private:
	void StartTCPServer( EventArgs* args );

private:
	bool m_isListening = false;
	int m_listenPort = -1;
	unsigned long long m_listenSocket;
};

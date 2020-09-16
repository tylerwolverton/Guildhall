#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Networking/TCPSocket.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class TCPClient;
class TCPServer;


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

	//std::string GetAddress();

private:
	void CheckForNewClientConnection();
	void ReceiveMessageFromServer();
	void ReceiveMessageFromClient();

	// Console commands
	void StartTCPServer( EventArgs* args );
	void StopTCPServer( EventArgs* args );
	void ConnectTCPClient( EventArgs* args );
	void DisconnectTCPClient( EventArgs* args );
	void DisconnectTCPServer( EventArgs* args );
	void SendMessage( EventArgs* args );

private:
	// Just one server for now, can be array later
	TCPServer* m_tcpServer = nullptr;
	TCPClient* m_tcpClient = nullptr;
	TCPSocket m_serverSocket;
	TCPSocket m_clientSocket;
	//std::vector<TCPClient*> m_tcpClients;


	/*bool m_isListening = false;
	FD_SET m_listenSet;
	timeval m_timeval;
	int m_listenPort = -1;
	SOCKET m_listenSocket = INVALID_SOCKET;
	SOCKET m_clientSocket = INVALID_SOCKET;*/
};

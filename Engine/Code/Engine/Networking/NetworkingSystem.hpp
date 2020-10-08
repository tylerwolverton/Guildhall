#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/SynchronizedBlockingQueue.hpp"
#include "Engine/Core/SynchronizedNonBlockingQueue.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/UDPSocket.hpp"

#include <map>
#include <string>
#include <thread>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct MessageHeader;
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
	// TCP
	void ProcessTCPCommunication();
	void CheckForNewTCPClientConnection();
	void ReceiveMessageFromTCPServer();
	void ReceiveMessageFromTCPClient();

	// UDP
	void UDPReaderThreadMain();
	void UDPWriterThreadMain();

	// Console commands
	void StartTCPServer( EventArgs* args );
	void StopTCPServer( EventArgs* args );
	void ConnectTCPClient( EventArgs* args );
	void DisconnectTCPClient( EventArgs* args );
	void DisconnectTCPServer( EventArgs* args );
	void SendMessage( EventArgs* args );

	void OpenUDPPort( EventArgs* args );
	void CloseUDPPort( EventArgs* args );
	void SendUDPMessage( EventArgs* args );

private:
	// Just one server for now, can be array later
	TCPServer* m_tcpServer = nullptr;
	TCPClient* m_tcpClient = nullptr;
	TCPSocket m_serverSocket;
	TCPSocket m_clientSocket;

	std::map<int, UDPSocket*> m_udpSockets;
	UDPSocket* m_udpSocket = nullptr;

	SynchronizedNonBlockingQueue<MessageHeader*> m_incomingMessages;
	SynchronizedNonBlockingQueue<MessageHeader*> m_outgoingMessages;
	//SynchronizedBlockingQueue<MessageHeader*> m_outgoingMessages;

	bool m_isQuitting = false;
	std::thread* m_udpReaderThread = nullptr;
	std::thread* m_udpWriterThread = nullptr;
};

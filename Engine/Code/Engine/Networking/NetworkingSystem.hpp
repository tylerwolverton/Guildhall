#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/SynchronizedBlockingQueue.hpp"
#include "Engine/Core/SynchronizedNonBlockingQueue.hpp"
#include "Engine/Networking/MessageProtocols.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/UDPSocket.hpp"

#include <array>
#include <map>
#include <string>
#include <thread>
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
	
	TCPSocket* ConnectTCPClientToServer( const std::string& host, int port );

	// TCP
	void StartTCPServer( int listenPort );
	void StopTCPServer();
	void ConnectTCPClient( const std::string& host, int port );
	void DisconnectTCPClient();
	void DisconnectTCPServer();
	void SendTCPMessage( void* data, size_t dataSize );
	void SendTCPTextMessage( const std::string& text );

	std::vector<TCPData> ReceiveTCPMessages();

	// UDP
	void OpenUDPPort( int localBindPort, int distantSendToPort );
	void CloseUDPPort( int localBindPort );
	void SendUDPMessage( int localBindPort, void* data, size_t dataSize );
	void SendUDPTextMessage( int localBindPort, const std::string& text );

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

	std::vector<TCPData> m_tcpReceivedMessages;


	std::map<int, UDPSocket*> m_udpSockets;
	UDPSocket* m_udpSocket = nullptr;

	SynchronizedNonBlockingQueue<UDPData> m_incomingMessages;
	SynchronizedBlockingQueue<std::array<char, 512>> m_outgoingMessages;

	bool m_isQuitting = false;
	std::thread* m_udpReaderThread = nullptr;
	std::thread* m_udpWriterThread = nullptr;
};

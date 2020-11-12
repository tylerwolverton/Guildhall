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
struct UDPMessage
{
	int sendToPort = -1;
	std::array<char, 512> data;

public:
	UDPMessage() = default;

	UDPMessage( int sendToPortIn, std::array<char, 512> dataIn )
		: sendToPort( sendToPortIn )
		, data( dataIn )
	{}

};


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
	std::vector<UDPData>& ReceiveUDPMessages();

	// UDP
	void OpenAndBindUDPPort( int localBindPort, int distantSendToPort );
	void CreateAndRegisterUDPSocket( int distantSendToPort );
	void CloseUDPPort( int localBindPort );
	void SendUDPMessage( int distantSendToPort, void* data, size_t dataSize );
	void SendUDPTextMessage( int localBindPort, const std::string& text );

private:
	// TCP
	void ProcessTCPCommunication();
	void CheckForNewTCPClientConnection();
	void ReceiveMessageFromTCPServer();
	void ReceiveMessageFromTCPClient();

	// UDP
	void ProcessUDPCommunication();
	void UDPReaderThreadMain();
	void UDPWriterThreadMain();
	void ClearProcessedUDPMessages();

	// Console commands
	void StartTCPServer( EventArgs* args );
	void StopTCPServer( EventArgs* args );
	void ConnectTCPClient( EventArgs* args );
	void DisconnectTCPClient( EventArgs* args );
	void DisconnectTCPServer( EventArgs* args );
	void SendMessage( EventArgs* args );

	void OpenAndBindUDPPort( EventArgs* args );
	void CloseUDPPort( EventArgs* args );
	void SendUDPMessage( EventArgs* args );

private:
	// Just one server for now, can be array later
	TCPServer* m_tcpServer = nullptr;
	TCPClient* m_tcpClient = nullptr;
	TCPSocket m_serverSocket;
	TCPSocket m_clientSocket;

	std::vector<TCPData> m_tcpReceivedMessages;
	std::vector<UDPData> m_udpReceivedMessages;


	std::map<int, UDPSocket*> m_udpSockets;
	UDPSocket* m_localBoundUDPSocket = nullptr;

	SynchronizedNonBlockingQueue<UDPData> m_incomingMessages;
	SynchronizedBlockingQueue<UDPMessage> m_outgoingMessages;

	bool m_isQuitting = false;
	std::thread* m_udpReaderThread = nullptr;
	std::thread* m_udpWriterThread = nullptr;
};

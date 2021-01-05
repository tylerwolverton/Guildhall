#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/SynchronizedBlockingQueue.hpp"
#include "Engine/Core/SynchronizedNonBlockingQueue.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Networking/MessageProtocols.hpp"
#include "Engine/Networking/TCPSocket.hpp"
#include "Engine/Networking/UDPSocket.hpp"

#include <array>
#include <map>
#include <string>
#include <thread>
#include <unordered_set>
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
struct ReliableUDPMessage
{
public:
	UDPMessage udpMessage;
	int retryCount = 0;
	int maxRetryCount = 1000;
	bool hasBeenAcked = false;

public:
	ReliableUDPMessage() = default;

	ReliableUDPMessage( UDPMessage udpMessage, int maxRetryCount )
		: udpMessage( udpMessage )
		, maxRetryCount( maxRetryCount )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct ReceivedUDPMessageId
{
public:
	UniqueMessageId uniqueId = 0;
	int port = 0;

public:
	ReceivedUDPMessageId() = default;
	~ReceivedUDPMessageId() = default;

	ReceivedUDPMessageId( UniqueMessageId uniqueId, int port )
		: uniqueId( uniqueId )
		, port( port )
	{
	}
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
	void OpenAndBindUDPPort( int localBindPort, int distantSendToPort, const std::string& ipAddress = "" );
	void CreateAndRegisterUDPSocket( int distantSendToPort, const std::string& ipAddress = "" );
	void CloseUDPPort( int localBindPort );
	void SendUDPMessage( int distantSendToPort, void* data, size_t dataSize, bool isReliable = false, int retryCount = 1000 );
	void SendUDPTextMessage( int localBindPort, const std::string& text );

private:
	// TCP
	void ProcessTCPCommunication();
	void CheckForNewTCPClientConnection();
	void ReceiveMessageFromTCPServer();
	void ReceiveMessageFromTCPClient();

	// UDP
	void ProcessUDPCommunication();
	void UDPReaderThreadMain( int localUDPPort );
	void UDPWriterThreadMain();
	void ClearProcessedUDPMessages();
	void RetryReliableUDPMessages();

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

	UniqueMessageId GetNextUniqueMessageId();

private:
	// Just one server for now, can be array later
	TCPServer* m_tcpServer = nullptr;
	TCPClient* m_tcpClient = nullptr;
	TCPSocket m_serverSocket;
	TCPSocket m_clientSocket;

	std::vector<TCPData> m_tcpReceivedMessages;
	std::vector<UDPData> m_udpReceivedMessages;

	std::map<int, UDPSocket*> m_outgoingUDPSockets;
	std::map<int, UDPSocket*> m_localBoundUDPSockets;
	//UDPSocket* m_localBoundUDPSocket = nullptr;

	SynchronizedNonBlockingQueue<UDPData> m_incomingMessages;
	SynchronizedBlockingQueue<UDPMessage> m_outgoingMessages;

	std::map<int, std::unordered_set<UniqueMessageId>> m_receivedReliableMessages;
	std::map<UniqueMessageId, ReliableUDPMessage> m_reliableUDPMessagesToRetry;

	bool m_isQuitting = false;
	std::thread* m_udpReaderThread = nullptr;
	std::vector<std::thread*> m_udpReaderThreads;
	std::thread* m_udpWriterThread = nullptr;

	RandomNumberGenerator m_rng;
};

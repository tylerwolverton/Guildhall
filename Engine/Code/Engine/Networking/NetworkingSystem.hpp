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
struct UDPPacket
{
	int sendToPort = -1;
	int numMessages = 0;
	int curSize = 0;
	std::array<char, 512> data;

public:
	UDPPacket() = default;

	UDPPacket( int sendToPortIn, std::array<char, 512> dataIn )
		: sendToPort( sendToPortIn )
		, data( dataIn )
	{}

};


//-----------------------------------------------------------------------------------------------
struct UDPMessage
{
public:
	std::array<char, 512> data;

public:
	UDPMessage() = default;

	UDPMessage( std::array<char, 512> data )
		: data( data )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct ReliableUDPMessage
{
public:
	UDPMessage udpMessage;
	int sendToPort = -1;
	int size = 0;
	int retryCount = 0;
	bool hasBeenAcked = false;

public:
	ReliableUDPMessage() = default;

	ReliableUDPMessage( int sendToPort, UDPMessage udpMessage, int msgSize )
		: udpMessage( udpMessage )
		, sendToPort( sendToPort )
		, size( msgSize )
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
	void OpenAndBindUDPPort( int localBindPort, int distantSendToPort );
	void CreateAndRegisterUDPSocket( int distantSendToPort );
	void CloseUDPPort( int localBindPort );
	void SendUDPMessage( int distantSendToPort, void* data, size_t dataSize, bool isReliable = false );
	void SendUDPTextMessage( int localBindPort, const std::string& text );

private:
	// TCP
	void ProcessTCPCommunication();
	void CheckForNewTCPClientConnection();
	void ReceiveMessageFromTCPServer();
	void ReceiveMessageFromTCPClient();

	// UDP
	void ProcessUDPCommunication();
	void ProcessIncomingUDPData( const UDPData& data );
	void AddMessageToOutgoingPacket( int distantSendToPort, const UDPMessage& msg, int msgSize );
	std::vector<UDPData> SplitUDPPacket( const UDPData& packet );
	void UDPReaderThreadMain();
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

	std::map<int, UDPSocket*> m_udpSockets;
	std::map<int, UDPPacket> m_udpOutgoingPackets;
	UDPSocket* m_localBoundUDPSocket = nullptr;

	SynchronizedNonBlockingQueue<UDPData> m_incomingMessages;
	SynchronizedBlockingQueue<UDPPacket> m_outgoingPackets;

	std::map<int, std::unordered_set<UniqueMessageId>> m_receivedReliableMessages;
	std::map<UniqueMessageId, ReliableUDPMessage> m_reliableUDPMessagesToRetry;

	bool m_isQuitting = false;
	std::thread* m_udpReaderThread = nullptr;
	std::thread* m_udpWriterThread = nullptr;

	RandomNumberGenerator m_rng;
};

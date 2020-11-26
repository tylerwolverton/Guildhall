#include "Engine/Networking/NetworkingSystem.hpp"
#include "Engine/Networking/MessageProtocols.hpp"
#include "Engine/Networking/TCPClient.hpp"
#include "Engine/Networking/TCPServer.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <array>
#include <winsock2.h>
#include <ws2tcpip.h>


//-----------------------------------------------------------------------------------------------
// Winsock Library link
//-----------------------------------------------------------------------------------------------
#pragma comment(lib, "Ws2_32.lib")


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::Startup()
{
	// tcp commands
	g_eventSystem->RegisterMethodEvent( "start_tcp_server", "Start TCP server listening for client connections, port=48000.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::StartTCPServer );
	g_eventSystem->RegisterMethodEvent( "stop_tcp_server",	"Stop TCP server listening for client connections.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::StopTCPServer );
	g_eventSystem->RegisterMethodEvent( "connect",			"Connect to TCP server on given host, host=<ip4 address>:<port number>.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::ConnectTCPClient );
	g_eventSystem->RegisterMethodEvent( "disconnect",		"Disconnect TCP client from TCP server.", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::DisconnectTCPClient );
	g_eventSystem->RegisterMethodEvent( "send_message",		"Send a message, msg=\"<message text>\"", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::SendMessage );
	
	// udp commands
	g_eventSystem->RegisterMethodEvent( "open_udp_port",	"Open a UDP port and specify target port, bindPort=<port number> sendToPort=<port number>", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::OpenAndBindUDPPort );
	g_eventSystem->RegisterMethodEvent( "close_udp_port",	"Close a UDP port, bindPort=<port number>", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::CloseUDPPort );
	g_eventSystem->RegisterMethodEvent( "send_udp_message", "Send a message, msg=\"<message text>\"", eUsageLocation::DEV_CONSOLE, this, &NetworkingSystem::SendUDPMessage );

	// Initialize winsock
	WSADATA wsaData;
	WORD wVersion MAKEWORD( 2, 2 );
	int iResult = WSAStartup( wVersion, &wsaData );
	if ( iResult != 0 )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSAStartup failed with '%i'", WSAGetLastError() ) );
	}
	
	m_tcpServer = new TCPServer( eBlockingMode::NONBLOCKING );
	m_tcpClient = new TCPClient( eBlockingMode::NONBLOCKING );

	m_udpReaderThread = new std::thread( &NetworkingSystem::UDPReaderThreadMain, this );
	m_udpWriterThread = new std::thread( &NetworkingSystem::UDPWriterThreadMain, this );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::BeginFrame()
{
	ProcessTCPCommunication();
	ProcessUDPCommunication();
	ClearProcessedUDPMessages();
	RetryReliableUDPMessages();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::EndFrame()
{
	for ( auto& packet : m_udpOutgoingPackets )
	{
		UDPPacket packetClone = packet.second;

		/*const UDPPacketHeader* udpPacketHeader = reinterpret_cast<const UDPPacketHeader*>( packetClone );
		udpPacketHeader->msgCount = packetClone.numMessages;*/

		m_outgoingPackets.Push( packetClone );

		packet.second.data.fill( '\0' );
		packet.second.curSize = 0;
		packet.second.numMessages = 0;
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::Shutdown()
{
	m_isQuitting = true;

	DisconnectTCPClient( nullptr );
	DisconnectTCPServer( nullptr );

	PTR_SAFE_DELETE( m_tcpClient );
	PTR_SAFE_DELETE( m_tcpServer );

	m_outgoingPackets.NotifyAll();
	
	for ( auto& udpSocket : m_udpSockets )
	{
		if ( udpSocket.second != nullptr )
		{
			udpSocket.second->Close();
			//PTR_SAFE_DELETE( m_udpSocket );
		}
	}

	if ( m_localBoundUDPSocket != nullptr )
	{
		m_localBoundUDPSocket->Close();
		PTR_SAFE_DELETE( m_localBoundUDPSocket );
	}

	m_udpReaderThread->join();
	m_udpWriterThread->join();

	PTR_MAP_SAFE_DELETE( m_udpSockets );
	PTR_SAFE_DELETE( m_udpReaderThread );
	PTR_SAFE_DELETE( m_udpWriterThread );

	int iResult = WSACleanup();
	if ( iResult == SOCKET_ERROR )
	{
		g_devConsole->PrintError( Stringf( "Networking System: WSACleanup failed with '%i'", WSAGetLastError() ) );
	}
}


//-----------------------------------------------------------------------------------------------
TCPSocket* NetworkingSystem::ConnectTCPClientToServer( const std::string& host, int port )
{
	TCPSocket* newSocket = m_tcpClient->ConnectAndCreateSocket( host, port );

	if ( newSocket != nullptr 
		 && newSocket->IsValid() )
	{
		std::string hostName( host );
		if ( hostName.empty() )
		{
			hostName = "localhost";
		}

		g_devConsole->PrintString( Stringf( "Connected to server '%s:%i'", hostName.c_str(), port ) );
	}

	return newSocket;
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ProcessTCPCommunication()
{
	// Check if we are a server that's listening
	if ( m_tcpServer->IsListening() )
	{
		if ( !m_serverSocket.IsValid() )
		{
			CheckForNewTCPClientConnection();
		}
		else
		{
			if ( m_serverSocket.IsDataAvailable() )
			{
				ReceiveMessageFromTCPClient();
			}
		}
	}
	// Check if we are a client
	else if ( m_clientSocket.IsValid() )
	{
		if ( m_clientSocket.IsDataAvailable() )
		{
			ReceiveMessageFromTCPServer();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::CheckForNewTCPClientConnection()
{
	m_serverSocket = m_tcpServer->Accept();

	if ( !m_serverSocket.IsValid() )
	{
		return;
	}

	// We found a new connection, send a server listening and print connection info
	g_devConsole->PrintString( Stringf( "Client connected from: %s", m_serverSocket.GetAddress().c_str() ) );

	std::string gameName( "Doomenstein" );
	std::array<char, 256> buffer;
	MessageHeader* msg = reinterpret_cast<MessageHeader*>( &buffer[0] );
	msg->id = (uint16_t)eMessasgeProtocolIds::SERVER_LISTENING;
	msg->size = ( uint16_t )gameName.size();

	memcpy( &buffer[4], gameName.c_str(), msg->size );

	m_serverSocket.Send( &buffer[0], gameName.size() + 4 );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ReceiveMessageFromTCPServer()
{
	TCPData data = m_clientSocket.Receive();
	if ( data.GetData() == nullptr )
	{
		return;
	}

	// Process message
	const MessageHeader* header = reinterpret_cast<const MessageHeader*>( data.GetData() );
	switch ( header->id )
	{
		case (uint16_t)eMessasgeProtocolIds::SERVER_LISTENING:
		{
			const char* dataStr = data.GetData() + 4;

			g_devConsole->PrintString( Stringf( "Connected to game: %s", dataStr ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::SERVER_DISCONNECTING:
		{
			m_clientSocket.Close();
			m_serverSocket.Close();
			g_devConsole->PrintString( Stringf( "Server disconnected" ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::TEXT:
		{
			const char* dataStr = data.GetData() + 4;

			g_devConsole->PrintString( Stringf( "Received from server: %s", dataStr ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::DATA:
		{
			m_tcpReceivedMessages.push_back( data );
		}
		break;

		default:
		{
			g_devConsole->PrintError( Stringf( "Received msg with unknown id: %i", header->id ) );
			return;
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ReceiveMessageFromTCPClient()
{
	TCPData data = m_serverSocket.Receive();
	if ( data.GetData() == nullptr )
	{
		return;
	}

	// Process message
	const MessageHeader* header = reinterpret_cast<const MessageHeader*>( data.GetData() );
	switch ( header->id )
	{
		case (uint16_t)eMessasgeProtocolIds::TEXT:
		{
			const char* dataStr = data.GetData() + 4;
			g_devConsole->PrintString( Stringf( "Received from client: %s", dataStr ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::CLIENT_DISCONNECTING:
		{
			m_clientSocket.Close();
			m_serverSocket.Close();
			g_devConsole->PrintString( Stringf( "Client disconnected" ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::DATA:
		{
			m_tcpReceivedMessages.push_back( data );
		}
		break;

		default:
		{
			g_devConsole->PrintError( Stringf( "Received msg with unknown id: %i", header->id ) );
			return;
		}
		break;	
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ProcessUDPCommunication()
{
	UDPData data;
	if ( !m_incomingMessages.Pop( data )
		 || data.GetLength() <= 0 )
	{
		return;
	}

	// Split data into individual data messages and process each one
	std::vector<UDPData> udpMessages = SplitUDPPacket( data );
	for( const auto& msg : udpMessages )
	{
		ProcessIncomingUDPData( msg );
	}
	//ProcessIncomingUDPData( data );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ProcessIncomingUDPData( const UDPData& data )
{
	const UDPMessageHeader* udpHeader = reinterpret_cast<const UDPMessageHeader*>( data.GetData() );
	switch ( udpHeader->id )
	{
		case (uint16_t)eMessasgeProtocolIds::TEXT:
		{
			g_devConsole->PrintString( Stringf( "Received message from '%s:': %s", data.GetFromAddress().c_str(), /*m_udpSocket->GetReceivePort(),*/ data.GetData() ) );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::DATA:
		{
			// This is a reliable message if it has a valid uniqueId
			UniqueMessageId uniqueMsgId = udpHeader->uniqueId;
			int distantToPort = udpHeader->localBindPort;
			if ( uniqueMsgId > 0 )
			{
				// Send an ack for the reliable message
				UDPMessageHeader ackHeader;
				ackHeader.id = (uint16_t)eMessasgeProtocolIds::ACK;
				ackHeader.size = (uint16_t)1;
				ackHeader.uniqueId = uniqueMsgId;
				ackHeader.localBindPort = m_localBoundUDPSocket->GetReceivePort();

				std::array<char, 512> buffer;
				memcpy( &buffer, &ackHeader, sizeof( UDPMessageHeader ) );
				buffer[sizeof( UDPMessageHeader ) + 1] = '\0';

				AddMessageToOutgoingPacket( distantToPort, UDPMessage( buffer ), sizeof( UDPMessageHeader ) + 1 );
				/*UDPPacket udpMessage( data.GetFromPort(), buffer );

				m_outgoingPackets.Push( udpMessage );*/

				// If we have already received this message, break to avoid processing it again
				const auto& receivedIter = m_receivedReliableMessages.find( distantToPort );
				if ( receivedIter != m_receivedReliableMessages.end() )
				{
					// This port has an entry, look for message id
					if ( receivedIter->second.find( uniqueMsgId ) != receivedIter->second.end() )
					{
						break;
					}

					// Add in new message id
					receivedIter->second.insert( uniqueMsgId );
				}
				else
				{
					// We haven't seen any messages from this port, create a new set with id and add it to map
					std::unordered_set<UniqueMessageId> uniqueIdSet = { uniqueMsgId };
					m_receivedReliableMessages[distantToPort] = uniqueIdSet;
				}

			}

			m_udpReceivedMessages.push_back( data );
		}
		break;

		case (uint16_t)eMessasgeProtocolIds::ACK:
		{
			// Remove message from retry list once an ack is received for it			
			UniqueMessageId ackId = udpHeader->uniqueId;

			m_reliableUDPMessagesToRetry.erase( ackId );
		}
		break;

		default:
		{
			g_devConsole->PrintError( Stringf( "Received msg with unknown id: %i", udpHeader->id ) );
			return;
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::AddMessageToOutgoingPacket( int distantSendToPort, const UDPMessage& msg, int msgSize )
{
	UDPPacket& packet = m_udpOutgoingPackets[distantSendToPort];
	if ( 512 - packet.curSize < msgSize )
	{
		g_devConsole->PrintWarning( "Packet too full for new message" );
		return;
	}

	memcpy( (void*)&(packet.data[packet.curSize]), (void*)(msg.data.data()), msgSize );

	++packet.numMessages;
	packet.curSize += msgSize;
}


//-----------------------------------------------------------------------------------------------
std::vector<UDPData> NetworkingSystem::SplitUDPPacket( const UDPData& packet )
{
	const UDPPacketHeader* udpPacketHeader = reinterpret_cast<const UDPPacketHeader*>( packet.GetData() );
	int numMsgs = udpPacketHeader->msgCount;

	std::vector<UDPData> msgs;

	int curPacketIdx = sizeof( UDPPacketHeader );
	for ( int curMsgNum = 0; curMsgNum < numMsgs; ++curMsgNum )
	{
		const UDPMessageHeader* udpHeader = reinterpret_cast<const UDPMessageHeader*>( packet.GetData() + curPacketIdx );
		UDPData msg( sizeof( UDPMessageHeader ) + udpHeader->size, packet.GetData() + curPacketIdx, packet.GetFromAddress(), packet.GetFromPort() );
		msgs.push_back( msg );
	}


	return msgs;
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::UDPReaderThreadMain()
{
	while( !m_isQuitting )
	{
		if ( m_localBoundUDPSocket == nullptr )
		{
			continue;
		}

		UDPData data = m_localBoundUDPSocket->Receive();
		if ( data.GetLength() > 0 )
		{
			m_incomingMessages.Push( data );
		}		
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::UDPWriterThreadMain()
{
	while ( !m_isQuitting )
	{
		UDPPacket message = m_outgoingPackets.Pop();
		UDPPacketHeader* msgHeader = reinterpret_cast<UDPPacketHeader*>( &message.data[0] );
		msgHeader->msgCount = message.numMessages;
		//UDPMessageHeader* msgHeader = reinterpret_cast<UDPMessageHeader*>( &message.data[0] );

		while ( msgHeader->size > 0 )
		{
			UDPSocket* udpSocket = nullptr;
			auto udpSocketIter = m_udpSockets.find( message.sendToPort );
			if ( udpSocketIter == m_udpSockets.end() )
			{
				// This is on a client with only 1 connection, send back on that socket
				if ( m_udpSockets.size() == 1 )
				{
					udpSocket = m_udpSockets.begin()->second;
				}
			}
			else
			{
				udpSocket = udpSocketIter->second;
			}

			// Get next message and retry
			if ( udpSocket == nullptr )
			{
				message = m_outgoingPackets.Pop();
				msgHeader = reinterpret_cast<UDPPacketHeader*>( &message.data[0] );
				//msgHeader = reinterpret_cast<UDPMessageHeader*>( &message.data[0] );

				continue;
			}

			// Copy the header and data into the buffer.
			udpSocket->SendBuffer() = message.data;

			udpSocket->Send( sizeof( UDPPacketHeader ) + msgHeader->size + 1 );
			//udpSocket->Send( sizeof( UDPPacketHeader ) + msgHeader->size + 1 );

			message = m_outgoingPackets.Pop();
			msgHeader = reinterpret_cast<UDPPacketHeader*>( &message.data[0] );
			//msgHeader = reinterpret_cast<UDPMessageHeader*>( &message.data[0] );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ClearProcessedUDPMessages()
{
	std::vector<int> indicesToDelete;

	for ( int udpMsgIdx = (int)m_udpReceivedMessages.size() - 1; udpMsgIdx >= 0; --udpMsgIdx )
	{
		if ( m_udpReceivedMessages[udpMsgIdx].HasBeenProcessed() )
		{
			indicesToDelete.push_back( udpMsgIdx );
		}
	}

	for ( int idx : indicesToDelete )
	{
		m_udpReceivedMessages.erase( m_udpReceivedMessages.begin() + idx );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::RetryReliableUDPMessages()
{
	std::vector<UniqueMessageId> expiredMessages;

	for ( auto& reliableMessage : m_reliableUDPMessagesToRetry )
	{
		++reliableMessage.second.retryCount;

		/*if ( reliableMessage.second.hasBeenAcked )
		{
			continue;
		}*/

		// Only start retrying after the message has been sent once by the usual means
		if ( reliableMessage.second.retryCount > 1 )
		{
			AddMessageToOutgoingPacket( reliableMessage.second.sendToPort, reliableMessage.second.udpMessage, reliableMessage.second.size );
			//m_outgoingPackets.Push( reliableMessage.second.udpMessage );
		}
		
		if ( reliableMessage.second.retryCount > 1000 )
		{
			//expiredMessages.push_back( reliableMessage.first );
		}
	}

	// Clean out old messages
	for ( const auto& expiredMessageId : expiredMessages )
	{
		m_reliableUDPMessagesToRetry.erase( expiredMessageId );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::StartTCPServer( EventArgs* args )
{
	int listenPort = args->GetValue( "port", 48000 );

	StartTCPServer( listenPort );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::StartTCPServer( int listenPort )
{
	if ( m_tcpServer == nullptr )
	{
		g_devConsole->PrintError( Stringf( "No TCPServer exists to start" ) );
		return;
	}

	m_tcpServer->Bind( listenPort );

	m_tcpServer->StartListening();

	g_devConsole->PrintString( Stringf( "Server listening on port '%i'", listenPort ) );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::StopTCPServer( EventArgs* args )
{
	UNUSED( args );

	StopTCPServer();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::StopTCPServer()
{
	if ( m_tcpServer == nullptr )
	{
		g_devConsole->PrintError( Stringf( "No TCPServer exists to stop" ) );
		return;
	}

	m_tcpServer->StopListening();

	DisconnectTCPServer( nullptr );

	g_devConsole->PrintString( Stringf( "Server stopped" ) );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ConnectTCPClient( EventArgs* args )
{
	std::string connectionAddress = args->GetValue( "host", ":48000" );

	Strings connectionComponents = SplitStringOnDelimiter( connectionAddress, ':' );

	if ( connectionComponents.size() != 2 )
	{
		g_devConsole->PrintError( Stringf( "Host must be in the form <ip4 address>:<port number>" ) );
		return;
	}

	ConnectTCPClient( connectionComponents[0], atoi( connectionComponents[1].c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::ConnectTCPClient( const std::string& host, int port )
{
	m_clientSocket = m_tcpClient->Connect( host, port );

	if ( m_clientSocket.IsValid() )
	{
		std::string hostName( host );
		if ( hostName.empty() )
		{
			hostName = "localhost";
		}

		g_devConsole->PrintString( Stringf( "Connected to server '%s:%i'", hostName.c_str(), port ) );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::DisconnectTCPClient( EventArgs* args )
{
	UNUSED( args );

	DisconnectTCPClient();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::DisconnectTCPClient()
{
	if ( m_clientSocket.IsValid() )
	{
		ClientDisconnectingMsg msg;
		msg.header.id = (uint16_t)eMessasgeProtocolIds::CLIENT_DISCONNECTING;
		msg.header.size = 0;

		m_clientSocket.Send( reinterpret_cast<char*>( &msg ), msg.GetSize() );

		//Sleep( 1000 );
	}

	m_tcpClient->Disconnect();

	m_clientSocket.Close();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::DisconnectTCPServer( EventArgs* args )
{
	UNUSED( args );

	DisconnectTCPServer();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::DisconnectTCPServer()
{
	if ( m_serverSocket.IsValid() )
	{
		ServerDisconnectingMsg msg;
		msg.header.id = (uint16_t)eMessasgeProtocolIds::SERVER_DISCONNECTING;
		msg.header.size = 0;

		m_serverSocket.Send( reinterpret_cast<char*>( &msg ), msg.GetSize() );
	}

	m_serverSocket.Close();
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendTCPMessage( void* data, size_t dataSize )
{
	std::array<char, 256> buffer;
	MessageHeader* msgHeader = reinterpret_cast<MessageHeader*>( &buffer[0] );

	msgHeader->id = (uint16_t)eMessasgeProtocolIds::DATA;
	msgHeader->size = (uint16_t)dataSize;

	memcpy( &buffer[4], data, msgHeader->size );

	// Send from server to clients
	if ( m_serverSocket.IsValid() )
	{
		m_serverSocket.Send( &buffer[0], msgHeader->size + 4 );
	}

	// Send from client to servers
	else if ( m_clientSocket.IsValid() )
	{
		m_clientSocket.Send( &buffer[0], msgHeader->size + 4 );
	}
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendTCPTextMessage( const std::string& text )
{
	std::array<char, 256> buffer;
	MessageHeader* msgHeader = reinterpret_cast<MessageHeader*>( &buffer[0] );

	msgHeader->id = (uint16_t)eMessasgeProtocolIds::TEXT;
	msgHeader->size = (uint16_t)text.size();

	memcpy( &buffer[4], text.c_str(), msgHeader->size );

	// Send from server to clients
	if ( m_serverSocket.IsValid() )
	{
		m_serverSocket.Send( &buffer[0], msgHeader->size + 4 );
	}

	// Send from client to servers
	else if ( m_clientSocket.IsValid() )
	{
		m_clientSocket.Send( &buffer[0], msgHeader->size + 4 );
	}
}


//-----------------------------------------------------------------------------------------------
std::vector<TCPData> NetworkingSystem::ReceiveTCPMessages()
{
	std::vector<TCPData> newMessages = m_tcpReceivedMessages;

	m_tcpReceivedMessages.clear();

	return newMessages;
}


//-----------------------------------------------------------------------------------------------
std::vector<UDPData>& NetworkingSystem::ReceiveUDPMessages()
{
	/*std::vector<UDPData> newMessages = m_udpReceivedMessages;

	m_udpReceivedMessages.clear();

	return newMessages;*/

	return m_udpReceivedMessages;
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendMessage( EventArgs* args )
{
	std::string msg = args->GetValue( "msg", "" );
	
	SendTCPTextMessage( msg );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::OpenAndBindUDPPort( EventArgs* args )
{
	int bindPort = args->GetValue( "bindPort", 48000 );
	int sendToPort = args->GetValue( "sendToPort", 48001 );
	
	OpenAndBindUDPPort( bindPort, sendToPort );

	/*if ( m_udpSocket != nullptr )
	{
		m_udpSocket->Close();
		PTR_SAFE_DELETE( m_udpSocket );
	}

	m_udpSocket = new UDPSocket( "", sendToPort );
	m_udpSocket->Bind( bindPort );*/
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::OpenAndBindUDPPort( int localBindPort, int distantSendToPort )
{
	UNUSED( distantSendToPort );
	//m_udpSockets[distantSendToPort] = new UDPSocket( "", distantSendToPort );
	//m_udpSockets[distantSendToPort]->Bind( localBindPort );

	m_localBoundUDPSocket = new UDPSocket( "", -1 );
	//m_udpSocket = new UDPSocket( "", distantSendToPort );
	m_localBoundUDPSocket->Bind( localBindPort );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::CreateAndRegisterUDPSocket( int distantSendToPort )
{
	m_udpSockets[distantSendToPort] = new UDPSocket( "", distantSendToPort );
	m_udpOutgoingPackets[distantSendToPort] = UDPPacket( distantSendToPort, std::array<char, 512>() );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::CloseUDPPort( EventArgs* args )
{
	int bindPort = args->GetValue( "bindPort", 48000 );

	CloseUDPPort( bindPort );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::CloseUDPPort( int localBindPort )
{
	auto udpSocketIter = m_udpSockets.find( localBindPort );
	if ( udpSocketIter == m_udpSockets.end() )
	{
		return;
	}
	UDPSocket* udpSocket = udpSocketIter->second;

	udpSocket->Close();
	PTR_SAFE_DELETE( udpSocket );

	m_udpSockets.erase( localBindPort );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendUDPMessage( EventArgs* args )
{
	std::string msg = args->GetValue( "msg", "" );

	SendUDPTextMessage( 48000, msg );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendUDPMessage( int distantSendToPort, void* data, size_t dataSize, bool isReliable )
{
	std::array<char, 512> buffer = {};
	UDPMessageHeader* msgHeader = reinterpret_cast<UDPMessageHeader*>( &buffer[0] );

	msgHeader->id = (uint16_t)eMessasgeProtocolIds::DATA;
	msgHeader->size = (uint16_t)dataSize;
	msgHeader->sequenceNum = (uint16_t)0;
	msgHeader->localBindPort = m_localBoundUDPSocket->GetReceivePort();

	if ( isReliable )
	{
		msgHeader->uniqueId = GetNextUniqueMessageId();
	}

	memcpy( &buffer[sizeof( UDPMessageHeader )], data, msgHeader->size );

	buffer[sizeof( UDPMessageHeader ) + msgHeader->size] = '\0';

	UDPMessage udpMessage( buffer );
	//UDPPacket udpMessage( distantSendToPort, buffer );
	if ( isReliable )
	{
		m_reliableUDPMessagesToRetry[msgHeader->uniqueId] = ReliableUDPMessage( distantSendToPort, udpMessage, sizeof( UDPMessageHeader ) + msgHeader->size );
	}

	AddMessageToOutgoingPacket( distantSendToPort, udpMessage, sizeof( UDPMessageHeader ) + msgHeader->size );
	//m_outgoingPackets.Push( udpMessage );
}


//-----------------------------------------------------------------------------------------------
UniqueMessageId NetworkingSystem::GetNextUniqueMessageId()
{
	return (UniqueMessageId)m_rng.RollRandomIntInRange( 1, 65535 );
}


//-----------------------------------------------------------------------------------------------
void NetworkingSystem::SendUDPTextMessage( int localBindPort, const std::string& text )
{
	std::array<char, 512> buffer = {};
	UDPMessageHeader* msgHeader = reinterpret_cast<UDPMessageHeader*>( &buffer[0] );

	msgHeader->id = (uint16_t)eMessasgeProtocolIds::TEXT;
	msgHeader->size = (uint16_t)text.size();
	msgHeader->sequenceNum = (uint16_t)0;
	msgHeader->localBindPort = m_localBoundUDPSocket->GetReceivePort();

	memcpy( &buffer[sizeof( UDPMessageHeader )], text.c_str(), msgHeader->size );

	buffer[sizeof( UDPMessageHeader ) + msgHeader->size] = '\0';

	m_outgoingPackets.Push( UDPPacket( localBindPort, buffer ) );
}

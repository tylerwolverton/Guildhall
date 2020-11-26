#pragma once
#include "Engine/Core/StringUtils.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
typedef uint16_t UniqueMessageId;


//-----------------------------------------------------------------------------------------------
enum class eMessasgeProtocolIds : std::uint16_t
{
	SERVER_LISTENING = 1,
	TEXT,
	CLIENT_DISCONNECTING,
	SERVER_DISCONNECTING,
	DATA,
	ACK,
};


//-----------------------------------------------------------------------------------------------
struct MessageHeader
{
	uint16_t id;
	uint16_t size;
};


//-----------------------------------------------------------------------------------------------
struct UDPPacketHeader
{
	uint16_t id = 0;
	uint16_t size = 0;
	uint16_t msgCount = 0;
	//int localBindPort = 0;
};


//-----------------------------------------------------------------------------------------------
struct UDPMessageHeader
{
	uint16_t id = 0;
	uint16_t size = 0;
	uint16_t sequenceNum = 0;
	uint16_t uniqueId = 0;
	int localBindPort = 0;
};


//-----------------------------------------------------------------------------------------------
struct ServerListeningMsg
{
	MessageHeader header;
	std::string gameName;

public:
	size_t GetSize()				{ return sizeof( header ) + gameName.size(); }
};


//-----------------------------------------------------------------------------------------------
struct ClientDisconnectingMsg
{
	MessageHeader header;

public:
	size_t GetSize()				{ return sizeof( header ); }
};


//-----------------------------------------------------------------------------------------------
struct ServerDisconnectingMsg
{
	MessageHeader header;

public:
	size_t GetSize() { return sizeof( header ); }
};

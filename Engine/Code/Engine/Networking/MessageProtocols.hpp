#pragma once
#include "Engine/Core/StringUtils.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
enum class eMessasgeProtocolIds : std::uint16_t
{
	SERVER_LISTENING = 1,
	TEXT,
	CLIENT_DISCONNECTING,
	SERVER_DISCONNECTING,
	DATA,
};


//-----------------------------------------------------------------------------------------------
struct MessageHeader
{
	uint16_t id;
	uint16_t size;
};


//-----------------------------------------------------------------------------------------------
struct UDPMessageHeader
{
	uint16_t id;
	uint16_t size;
	uint16_t sequenceNum;
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
struct TextMsg
{
	MessageHeader header;
	std::string msg;

public:
	size_t GetSize()				{ return sizeof( header ) + msg.size(); }
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

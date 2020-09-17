#pragma once
#include "Engine/Core/StringUtils.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
enum class eMessasgeProtocolIds : std::uint16_t
{
	SERVER_LISTENING = 1,
	TEXT = 2,
	CLIENT_DISCONNECTING = 3,
	SERVER_DISCONNECTING = 4
};


//-----------------------------------------------------------------------------------------------
struct MessageHeader
{
	uint16_t id;
	uint16_t size;
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

#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
enum class eMessasgeProtocolIds : std::uint16_t
{
	SERVER_LISTENING = 1,
	TEXT = 2,
	CLIENT_DISCONNECTING = 3
};


//-----------------------------------------------------------------------------------------------
struct MessageHeader
{
	std::uint16_t id;
	std::uint16_t size;
};


//-----------------------------------------------------------------------------------------------
struct ServerListeningMsg
{
	MessageHeader header;
	std::string gameName;

};


//-----------------------------------------------------------------------------------------------
struct TextMsg
{
	MessageHeader header;
	std::string msg;
};


//-----------------------------------------------------------------------------------------------
struct ClientDisconnectingMsg
{
	MessageHeader header;
};

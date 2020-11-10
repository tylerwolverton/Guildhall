#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"
#include "Engine/Networking/MessageProtocols.hpp"

#include <array>
#include <string>


//-----------------------------------------------------------------------------------------------
constexpr int BUFFER_SIZE = 512;


//-----------------------------------------------------------------------------------------------
class UDPData
{
public:
	UDPData() = default;
	UDPData( size_t length, char* dataPtr, const std::string& fromAddress )
		: m_length( length )
		, m_data( dataPtr )
		, m_fromAddress( fromAddress )
	{
	}

	~UDPData() = default;

	size_t		GetLength() const				{ return m_length; }
	char*		GetData() const					{ return m_data; }
	const char* GetPayload() const				{ return m_data + sizeof( UDPMessageHeader ); }

	std::string GetFromAddress() const			{ return m_fromAddress; }
	std::string GetFromIPAddress() const;
	int			GetFromPort() const;

	bool		HasBeenProcessed()				{ return m_hasBeenProcessed; }
	void		Process()						{ m_hasBeenProcessed = true; }

private:
	size_t m_length = 0;
	char* m_data = nullptr;
	std::string m_fromAddress;
	bool m_hasBeenProcessed = false;
};


//-----------------------------------------------------------------------------------------------
class UDPSocket
{
public:
	UDPSocket( const std::string& host, int port );
	UDPSocket();
	~UDPSocket();

	void Bind( int port );
	void Close();
	int Send( size_t length );
	//int Send( const char* data, size_t length );
	UDPData Receive();

	std::array<char, BUFFER_SIZE>& SendBuffer()			{ return m_sendBuffer; }
	std::array<char, BUFFER_SIZE>& ReceiveBuffer()		{ return m_receiveBuffer; }

	int			GetReceivePort() const					{ return m_bindPort; }

private:
	std::array<char, BUFFER_SIZE> m_sendBuffer;
	std::array<char, BUFFER_SIZE> m_receiveBuffer;
	sockaddr_in m_toAddress;
	sockaddr_in m_bindAddress;
	SOCKET m_socket = INVALID_SOCKET;
	int m_bindPort = -1;
};

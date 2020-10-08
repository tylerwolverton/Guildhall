#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"

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
	std::string GetFromAddress() const			{ return m_fromAddress; }

	std::string GetDataAsString() const			{ return std::string( m_data, m_length ); }

private:
	size_t m_length = 0;
	char* m_data = nullptr;
	std::string m_fromAddress;
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
	int Send( const char* data, size_t length );
	UDPData Receive();

	std::array<char, BUFFER_SIZE>& sendBuffer()			{ return m_sendBuffer; }
	std::array<char, BUFFER_SIZE>& receiveBuffer()		{ return m_receiveBuffer; }

private:
	std::array<char, BUFFER_SIZE> m_sendBuffer;
	std::array<char, BUFFER_SIZE> m_receiveBuffer;
	sockaddr_in m_toAddress;
	sockaddr_in m_bindAddress;
	SOCKET m_socket = INVALID_SOCKET;
};

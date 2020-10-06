#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"

#include <array>
#include <string>


//-----------------------------------------------------------------------------------------------
constexpr int BUFFER_SIZE = 512;


//-----------------------------------------------------------------------------------------------
class UDPSocket
{
public:
	UDPSocket( const std::string& host, int port );
	UDPSocket();
	~UDPSocket();

	void Bind( int port );
	void Close();
	int Send( int length );
	int Receive();

	//std::array<char, BUFFER_SIZE> 

private:
	std::array<char, BUFFER_SIZE> m_sendBuffer;
	std::array<char, BUFFER_SIZE> m_receiveBuffer;
	sockaddr_in m_toAddress;
	sockaddr_in m_bindAddress;
	SOCKET m_socket = INVALID_SOCKET;
};

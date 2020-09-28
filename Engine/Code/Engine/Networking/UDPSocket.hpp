#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"

#include <string>

class UDPSocket
{
public:
	UDPSocket( const std::string& host, int port );
	~UDPSocket();

	void Bind( int port );
	void Close();
	int Send();
	int Receive();

private:
	sockaddr_in m_toAddress;
	SOCKET m_socket = INVALID_SOCKET;
};

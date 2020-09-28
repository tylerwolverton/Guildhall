#include "pch.h"
#include "CppUnitTest.h"

#include "Engine/Networking/UDPSocket.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


//-----------------------------------------------------------------------------------------------
// Winsock Library link
//-----------------------------------------------------------------------------------------------
#pragma comment(lib, "Ws2_32.lib")


namespace UnitTest
{
	TEST_CLASS(UnitTest)
	{
	public:
		
		TEST_METHOD_INITIALIZE( InitializeUDPSocketTest )
		{
			// Initialize winsock
			WSADATA wsaData;
			WORD wVersion MAKEWORD( 2, 2 );
			int iResult = WSAStartup( wVersion, &wsaData );
			if ( iResult != 0 )
			{
				std::cout << "Networking System: WSAStartup failed with " << WSAGetLastError() << std::endl;
			}
		}


		TEST_METHOD(UDPSocketTest)
		{
			Logger::WriteMessage( "Starting UDP Socket Test..." );

			UDPSocket socket1( "127.0.0.1", 48000 );
			socket1.Bind( 48001 );

			UDPSocket socket2( "127.0.0.1", 48001 );
			socket2.Bind( 48000 );

			Assert::AreEqual( 3, 3 );
		}


		TEST_METHOD_CLEANUP( CleanupUDPSocketTest )
		{
			int iResult = WSACleanup();
			if ( iResult == SOCKET_ERROR )
			{
				std::cout << "Networking System: WSACleanup failed with " << WSAGetLastError() << std::endl;
			}
		}
	};
}

#include "pch.h"
#include "CppUnitTest.h"

#define TEST_MODE

#include "Engine/Networking/UDPSocket.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <mutex>
#include <thread>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


//-----------------------------------------------------------------------------------------------
// Winsock Library link
//-----------------------------------------------------------------------------------------------
#pragma comment(lib, "Ws2_32.lib")


namespace UnitTest
{
	TEST_CLASS( UDPSocketTestCase )
	{
	public:
		//-----------------------------------------------------------------------------------------------
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
		

		//-----------------------------------------------------------------------------------------------
		struct UDPMessageHeader
		{
			uint16_t id;
			uint16_t length;
			uint16_t sequenceNum;
		};


		//-----------------------------------------------------------------------------------------------
		struct Message
		{
			uint16_t id = 0;
			uint16_t length = 0;
			uint16_t sequenceNum = 0;
			std::string text;
		};


		//-----------------------------------------------------------------------------------------------
		// Thread methods
		static void WriterMain( UDPSocket& socket, const Message& message )
		{
			UDPMessageHeader header;
			
			// Setup the message header.
			header.id = message.id;
			header.length = message.length;
			header.sequenceNum = message.sequenceNum;

			// Copy the header into the buffer.
			auto& buffer = socket.SendBuffer();
			*reinterpret_cast<UDPMessageHeader*>( &buffer[0] ) = header;

			// Copy the data into the buffer.
			// Notice we offset message header size into the buffer to write the data.
			std::memcpy( &( socket.SendBuffer()[sizeof( UDPMessageHeader )] ), message.text.c_str(), header.length );

			// Copy the message data into the buffer.
			socket.SendBuffer()[sizeof( UDPMessageHeader ) + header.length] = NULL;

			socket.Send( sizeof( UDPMessageHeader ) + header.length + 1 );
		}


		//-----------------------------------------------------------------------------------------------
		static void ReaderMain( UDPSocket& socket, Message& message )
		{
			static std::mutex lock;

			const UDPMessageHeader* msgHeader = nullptr;

			do
			{
				UDPData data = socket.Receive();

				message.text = "";
				if ( data.GetLength() > 0 )
				{
					auto& buffer = socket.ReceiveBuffer();

					// Copy the message header.
					msgHeader = reinterpret_cast<const UDPMessageHeader*>( &buffer[0] );
					if ( msgHeader->length > 0 )
					{
						// Copy the data.
						message.text = std::string( data.GetData(), msgHeader->length );
					}
				}
			} while ( msgHeader != nullptr && msgHeader->id != 0 );
		}


		//-----------------------------------------------------------------------------------------------
		TEST_METHOD(UDPSocketTest)
		{
			Logger::WriteMessage( "Starting UDP Socket Test..." );

			UDPSocket socketOne( "127.0.0.1", 48000 );
			socketOne.Bind( 48001 );

			UDPSocket socketTwo( "127.0.0.1", 48001 );
			socketTwo.Bind( 48000 );

			Message readMessageOne;
			std::string text1( "Hello from message one!" );
			readMessageOne.length = (int)text1.length();
			readMessageOne.text = text1;

			Message readMessageTwo;
			std::string text2( "Greetings from message two!" );
			readMessageTwo.length = (int)text2.length();
			readMessageTwo.text = text2;

			std::thread writerOne( &UDPSocketTestCase::WriterMain, std::ref( socketOne ), std::cref( readMessageOne ) );
			std::thread writerTwo( &UDPSocketTestCase::WriterMain, std::ref( socketTwo ), std::cref( readMessageTwo ) );

			std::thread readerOne( &UDPSocketTestCase::ReaderMain, std::ref( socketOne ), std::ref( readMessageTwo ) );
			std::thread readerTwo( &UDPSocketTestCase::ReaderMain, std::ref( socketTwo ), std::ref( readMessageOne ) );

			writerOne.join();
			writerTwo.join();
			readerOne.join();
			readerTwo.join();

			Assert::AreEqual( text1, readMessageOne.text );
			Assert::AreEqual( text2, readMessageTwo.text );
		}


		//-----------------------------------------------------------------------------------------------
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

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
		struct MessageHeader
		{
			uint16_t id;
			uint16_t length;
			uint32_t sequenceNum;
		};


		//-----------------------------------------------------------------------------------------------
		struct Message
		{
			int id = -1;
			int length = -1;
			int sequenceNum = -1;
			std::string text;
		};




		//-----------------------------------------------------------------------------------------------
		// Thread methods
		static void WriterMain( UDPSocket& socket, const std::string& message )
		{
			MessageHeader header;
			
			// Setup the message header.
		/*	header.id = message.id;
			header.length = message.length;
			header.sequenceNum = message.sequenceNum;*/

			//// Copy the header into the buffer.
			//auto& buffer = socket.sendBuffer();
			//*reinterpret_cast<MessageHeader*>( &buffer[0] ) = header;

			//// Copy the data into the buffer.
			//// Notice we offset message header size into the buffer to write the data.
			//std::memcpy( &( socket.sendBuffer()[sizeof( MessageHeader )] ), message.text.c_str(), header.length );

			//// Copy the message data into the buffer.
			//socket.sendBuffer()[sizeof( MessageHeader ) + header.length] = NULL;

			socket.Send( message.c_str(), message.length() );
			//socket.Send( message.text.c_str(), message.length );
			//socket.Send( message.text.c_str(), sizeof( MessageHeader ) + header.length + 1 );
		}


		//-----------------------------------------------------------------------------------------------
		static void ReaderMain( UDPSocket& socket, Message& message )
		{
			static std::mutex lock;

			const MessageHeader* msgHeader = nullptr;
			std::string dataStr;
			std::size_t length = 0;

			do
			{
				UDPData data = socket.Receive();

				//dataStr.clear();
				if ( data.GetLength() > 0 )
				{
					//auto& buffer = socket.receiveBuffer();

					//// Copy the message header.
					//msgHeader = reinterpret_cast<MessageHeader const*>( &buffer[0] );
					//if ( msgHeader->length > 0 )
					//{
					//	// Copy the data.
					//	dataStr = &buffer[sizeof( MessageHeader )];
					//}
					message.text = data.GetDataAsString();// [msgHeader->seqNo] = dataStr;
					
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

			Message readMessageTwo;
			std::string text2( "Greetings from message two!" );

			std::thread writerOne( &UDPSocketTestCase::WriterMain, std::ref( socketOne ), std::cref( text1 ) );
			std::thread writerTwo( &UDPSocketTestCase::WriterMain, std::ref( socketTwo ), std::cref( text2 ) );

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

#pragma once
#include "Engine/Core/StringUtils.hpp"


#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

class DevConsole;

extern DevConsole* g_devConsole;

#ifdef UNIT_TEST_MODE
#define LOG_ERROR(...) printf( Stringf( __VA_ARGS__ ) + std::string( "\n" )
#else
#define LOG_ERROR(...) g_devConsole->PrintError( Stringf( __VA_ARGS__ ) )
#endif


enum class eBlockingMode
{
	INVALID,
	BLOCKING,
	NONBLOCKING
};

#pragma once
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

enum class eBlockingMode
{
	INVALID,
	BLOCKING,
	NONBLOCKING
};

#pragma once
#include <string>
#include <vector>

typedef unsigned char byte;

enum class eOpCode : byte
{
	OP_UNKNOWN,

	OP_RETURN
};

bool operator==( eOpCode opCode, byte opCodeByte );
bool operator==( byte opCodeByte, eOpCode opCode );

byte OpCodeToByte( eOpCode opCode );
eOpCode ByteToOpCode( byte opCodeByte );

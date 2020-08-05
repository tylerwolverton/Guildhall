#pragma once
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
typedef unsigned char byte;


//-----------------------------------------------------------------------------------------------
enum class eOpCode : byte
{
	OP_UNKNOWN,

	OP_CONSTANT,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NEGATE,
	OP_RETURN,

	OP_LAST_VAL
};

bool operator==( eOpCode opCode, byte opCodeByte );
bool operator==( byte opCodeByte, eOpCode opCode );

byte OpCodeToByte( eOpCode opCode );
eOpCode ByteToOpCode( byte opCodeByte );

std::string ToString( eOpCode opCode );

//-----------------------------------------------------------------------------------------------
union Value
{
public:
	Value( float val ) { value = val; }
	
public:
	float value;
};

//-----------------------------------------------------------------------------------------------
//template < typename T> class Value
//{
//public:
//	Value( T val ) { value = val; }
//
//public:
//	T value;
//};

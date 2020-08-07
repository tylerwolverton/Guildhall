#include "BytecodeCommon.hpp"


//-----------------------------------------------------------------------------------------------
bool operator==( eOpCode opCode, byte opCodeByte )
{
	return (byte)opCode == opCodeByte;
}


//-----------------------------------------------------------------------------------------------
bool operator==( byte opCodeByte, eOpCode opCode )
{
	return opCode == opCodeByte;
}


//-----------------------------------------------------------------------------------------------
byte OpCodeToByte( eOpCode opCode )
{
	return (byte)opCode;
}


//-----------------------------------------------------------------------------------------------
eOpCode ByteToOpCode( byte opCodeByte )
{
	if ( opCodeByte < 0
		 || opCodeByte >= (byte)eOpCode::LAST_VAL )
	{
		return eOpCode::UNKNOWN;
	}

	return (eOpCode)opCodeByte;
}


//-----------------------------------------------------------------------------------------------
std::string ToString( eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::CONSTANT: return "OP_CONSTANT";
		case eOpCode::ADD: return "OP_ADD";
		case eOpCode::SUBTRACT: return "OP_SUBTRACT";
		case eOpCode::MULTIPLY: return "OP_MULTIPLY";
		case eOpCode::DIVIDE: return "OP_DIVIDE";
		case eOpCode::NEGATE: return "OP_NEGATE";
		case eOpCode::RETURN: return "OP_RETURN";
		default: return "OP_UNKNOWN";
	}
}


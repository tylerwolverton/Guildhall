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
		 || opCodeByte >= (byte)eOpCode::OP_LAST_VAL )
	{
		return eOpCode::OP_UNKNOWN;
	}

	return (eOpCode)opCodeByte;
}


//-----------------------------------------------------------------------------------------------
std::string ToString( eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::OP_CONSTANT: return "OP_CONSTANT";
		case eOpCode::OP_ADD: return "OP_ADD";
		case eOpCode::OP_SUBTRACT: return "OP_SUBTRACT";
		case eOpCode::OP_MULTIPLY: return "OP_MULTIPLY";
		case eOpCode::OP_DIVIDE: return "OP_DIVIDE";
		case eOpCode::OP_NEGATE: return "OP_NEGATE";
		case eOpCode::OP_RETURN: return "OP_RETURN";
		default: return "OP_UNKNOWN";
	}
}


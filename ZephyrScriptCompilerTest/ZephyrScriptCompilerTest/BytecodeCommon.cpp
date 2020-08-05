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
	if ( opCodeByte == eOpCode::OP_CONSTANT ) { return eOpCode::OP_CONSTANT; }
	else if ( opCodeByte == eOpCode::OP_RETURN ) { return eOpCode::OP_RETURN; }
	else { return eOpCode::OP_UNKNOWN; }
}

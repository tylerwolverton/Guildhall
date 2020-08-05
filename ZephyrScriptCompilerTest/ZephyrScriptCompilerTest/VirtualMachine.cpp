#include "VirtualMachine.hpp"

#include <iostream>


//-----------------------------------------------------------------------------------------------
void VirtualMachine::Interpret( const Chunk& chunk )
{
	int chunkIdx = 0;

	while ( true )
	{
		byte instruction = chunk.GetByte( chunkIdx++ );
		switch ( ByteToOpCode( instruction ) )
		{
			case eOpCode::OP_CONSTANT:
			{
				int constantIdx = chunk.GetByte( chunkIdx++ );
				Value constant = chunk.GetConstant( constantIdx );
				std::cout << "Constant val = " << constant.value << std::endl;
			}
			break;

			case eOpCode::OP_RETURN:
			{
				return;
			}
			break;

			default:
			{

			}
		}
	}
}


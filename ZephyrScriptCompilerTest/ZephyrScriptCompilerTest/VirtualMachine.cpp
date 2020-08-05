#include "VirtualMachine.hpp"

#include <iostream>


//-----------------------------------------------------------------------------------------------
void VirtualMachine::Interpret( const Chunk& chunk )
{
	int chunkIdx = 0;

	while ( true )
	{
		byte instruction = chunk.GetByte( chunkIdx++ );
		eOpCode opCode = ByteToOpCode( instruction );
		switch ( opCode )
		{
			case eOpCode::OP_CONSTANT:
			{
				int constantIdx = chunk.GetByte( chunkIdx++ );
				Value constant = chunk.GetConstant( constantIdx );
				PushValue( constant );
			}
			break;

			case eOpCode::OP_ADD:
			case eOpCode::OP_SUBTRACT:
			case eOpCode::OP_MULTIPLY:
			case eOpCode::OP_DIVIDE:
			{
				Value b = PopValue();
				Value a = PopValue();
				PushBinaryOp( a, b, opCode );
			}
			break;

			case eOpCode::OP_NEGATE:
			{
				Value negativeVal = PopValue();
				negativeVal.value = -negativeVal.value;
				PushValue( negativeVal );
			}
			break;

			case eOpCode::OP_RETURN:
			{
				std::cout << "Constant val = " << PopValue().value << std::endl;
				return;
			}
			break;

			default:
			{
				std::cout << "Unknown instruction ' " << instruction << "'\n";
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void VirtualMachine::PushValue( Value val )
{
	m_stack.push( val );
}


//-----------------------------------------------------------------------------------------------
Value VirtualMachine::PopValue()
{
	Value topOfStack = m_stack.top();
	m_stack.pop();

	return topOfStack;
}


//-----------------------------------------------------------------------------------------------
void VirtualMachine::PushBinaryOp( Value a, Value b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::OP_ADD:
		{
			float result = a.value + b.value;
			PushValue( Value( result ) );
		}
		break;

		case eOpCode::OP_SUBTRACT:
		{
			float result = a.value - b.value;
			PushValue( Value( result ) );
		}
		break;

		case eOpCode::OP_MULTIPLY:
		{
			float result = a.value * b.value;
			PushValue( Value( result ) );
		}
		break;

		case eOpCode::OP_DIVIDE:
		{
			float result = a.value / b.value;
			PushValue( Value( result ) );
		}
		break;

		default:
		{
			std::cout << "Tried to call BinaryOp on non binary operator ' " << ToString( opCode ) << "'\n";
		}
	}
}



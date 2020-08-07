#include "VirtualMachine.hpp"
#include "Compiler.hpp"
#include "Scanner.hpp"

#include <iostream>


//-----------------------------------------------------------------------------------------------
eInterpretResult VirtualMachine::Interpret( const Chunk& chunk )
{
	int chunkIdx = 0;

	while ( true )
	{
		byte instruction = chunk.GetByte( chunkIdx++ );
		eOpCode opCode = ByteToOpCode( instruction );
		switch ( opCode )
		{
			case eOpCode::CONSTANT:
			{
				int constantIdx = chunk.GetByte( chunkIdx++ );
				Value constant = chunk.GetConstant( constantIdx );
				PushValue( constant );
			}
			break;

			case eOpCode::ADD:
			case eOpCode::SUBTRACT:
			case eOpCode::MULTIPLY:
			case eOpCode::DIVIDE:
			{
				Value b = PopValue();
				Value a = PopValue();
				PushBinaryOp( a, b, opCode );
			}
			break;

			case eOpCode::NEGATE:
			{
				Value negativeVal = PopValue();
				negativeVal.value = -negativeVal.value;
				PushValue( negativeVal );
			}
			break;

			case eOpCode::RETURN:
			{
				std::cout << "Constant val = " << PopValue().value << std::endl;
				return eInterpretResult::OK;
			}
			break;

			default:
			{
				std::cout << "Unknown instruction ' " << instruction << "'\n";
				return eInterpretResult::COMPILE_ERROR;
			}
		}
	}

	return eInterpretResult::OK;
}


//-----------------------------------------------------------------------------------------------
eInterpretResult VirtualMachine::Interpret( const std::string& file )
{


	return eInterpretResult::OK;
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
		case eOpCode::ADD:
		{
			float result = a.value + b.value;
			PushValue( Value( result ) );
		}
		break;

		case eOpCode::SUBTRACT:
		{
			float result = a.value - b.value;
			PushValue( Value( result ) );
		}
		break;

		case eOpCode::MULTIPLY:
		{
			float result = a.value * b.value;
			PushValue( Value( result ) );
		}
		break;

		case eOpCode::DIVIDE:
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



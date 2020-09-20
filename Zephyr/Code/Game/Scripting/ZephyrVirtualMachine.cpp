#include "Game/Scripting/ZephyrVirtualMachine.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrVirtualMachine::ZephyrVirtualMachine()
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::Startup()
{

}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::Shutdown()
{

}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk )
{
	ClearNumberStack();

	int byteIdx = 0;
	while ( byteIdx < bytecodeChunk.GetNumBytes() )
	{
		byte instruction = bytecodeChunk.GetByte( byteIdx++ );
		eOpCode opCode = ByteToOpCode( instruction );
		switch ( opCode )
		{
			case eOpCode::CONSTANT_NUMBER:
			{
				int numConstIdx = bytecodeChunk.GetByte( byteIdx++ );
				NUMBER_TYPE numConstant = bytecodeChunk.GetNumberConstant( numConstIdx );
				PushNumber( numConstant );
			}
			break;

			case eOpCode::ADD:
			case eOpCode::SUBTRACT:
			case eOpCode::MULTIPLY:
			case eOpCode::DIVIDE:
			{
				// TODO: Support string concatenation?
				NUMBER_TYPE b = PopNumber();
				NUMBER_TYPE a = PopNumber();
				PushNumberBinaryOp( a, b, opCode );
			}
			break;

			default:
			{
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushNumber( NUMBER_TYPE number )
{
	m_numberStack.push( number );
}


//-----------------------------------------------------------------------------------------------
NUMBER_TYPE ZephyrVirtualMachine::PopNumber()
{
	NUMBER_TYPE topNum = m_numberStack.top();
	m_numberStack.pop();

	return topNum;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushNumberBinaryOp( NUMBER_TYPE a, NUMBER_TYPE b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::ADD:
		{
			NUMBER_TYPE result = a + b;
			PushNumber( result );
		}
		break;

		case eOpCode::SUBTRACT:
		{
			NUMBER_TYPE result = a - b;
			PushNumber( result );
		}
		break;

		case eOpCode::MULTIPLY:
		{
			NUMBER_TYPE result = a * b;
			PushNumber( result );
		}
		break;

		case eOpCode::DIVIDE:
		{
			NUMBER_TYPE result = a / b;
			PushNumber( result );
		}
		break;

		default:
		{

		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::ClearNumberStack()
{
	while ( !m_numberStack.empty() )
	{
		m_numberStack.pop();
	}
}

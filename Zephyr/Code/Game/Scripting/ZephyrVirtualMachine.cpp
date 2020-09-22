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
	ClearConstantStack();

	int byteIdx = 0;
	while ( byteIdx < bytecodeChunk.GetNumBytes() )
	{
		byte instruction = bytecodeChunk.GetByte( byteIdx++ );
		eOpCode opCode = ByteToOpCode( instruction );
		switch ( opCode )
		{
			case eOpCode::CONSTANT_NUMBER:
			{
				int constIdx = bytecodeChunk.GetByte( byteIdx++ );
				ZephyrValue constant = bytecodeChunk.GetConstant( constIdx );
				PushConstant( constant );
			}
			break;

			case eOpCode::NEGATE:
			{
				ZephyrValue a = PopConstant();
				if ( a.GetType() == eValueType::NUMBER )
				{
					PushConstant( -a.GetAsNumber() );
				}
			}
			break;

			case eOpCode::ADD:
			case eOpCode::SUBTRACT:
			case eOpCode::MULTIPLY:
			case eOpCode::DIVIDE:
			{
				// TODO: Support string concatenation?
				ZephyrValue b = PopConstant();
				ZephyrValue a = PopConstant();
				PushBinaryOp( a, b, opCode );
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
void ZephyrVirtualMachine::PushConstant( const ZephyrValue& number )
{
	m_constantStack.push( number );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::PopConstant()
{
	ZephyrValue topConstant = m_constantStack.top();
	m_constantStack.pop();

	return topConstant;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushBinaryOp( const ZephyrValue& a, const ZephyrValue& b, eOpCode opCode )
{
	if ( a.GetType() == eValueType::NUMBER
		 && b.GetType() == eValueType::NUMBER )
	{
		PushNumberBinaryOp( a.GetAsNumber(), b.GetAsNumber(), opCode );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushNumberBinaryOp( NUMBER_TYPE a, NUMBER_TYPE b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::ADD:
		{
			NUMBER_TYPE result = a + b;
			PushConstant( result );
		}
		break;

		case eOpCode::SUBTRACT:
		{
			NUMBER_TYPE result = a - b;
			PushConstant( result );
		}
		break;

		case eOpCode::MULTIPLY:
		{
			NUMBER_TYPE result = a * b;
			PushConstant( result );
		}
		break;

		case eOpCode::DIVIDE:
		{
			NUMBER_TYPE result = a / b;
			PushConstant( result );
		}
		break;

		default:
		{

		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::ClearConstantStack()
{
	while ( !m_constantStack.empty() )
	{
		m_constantStack.pop();
	}
}

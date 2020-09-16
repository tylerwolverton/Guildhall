#include "Game/Scripting/ZephyrVirtualMachine.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"


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
void ZephyrVirtualMachine::ClearNumberStack()
{
	while ( !m_numberStack.empty() )
	{
		m_numberStack.pop();
	}
}

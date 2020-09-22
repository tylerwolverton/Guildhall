#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <stack>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;


//-----------------------------------------------------------------------------------------------
class ZephyrVirtualMachine
{
public:
	ZephyrVirtualMachine();

	void Startup();
	void Shutdown();

	void InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk );

private:
	void		PushConstant( const ZephyrValue& number );
	ZephyrValue PopConstant();

	void PushBinaryOp( const ZephyrValue& a, const ZephyrValue& b, eOpCode opCode );
	void PushNumberBinaryOp( NUMBER_TYPE a, NUMBER_TYPE b, eOpCode opCode );

	void ClearNumberStack();

private:
	std::stack<ZephyrValue> m_constantStack;
};

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
	void		PushNumber( NUMBER_TYPE number );
	NUMBER_TYPE PopNumber();

	void ClearNumberStack();

private:
	std::stack<NUMBER_TYPE> m_numberStack;
};

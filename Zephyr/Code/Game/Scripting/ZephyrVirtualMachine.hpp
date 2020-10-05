#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <stack>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class Entity;


//-----------------------------------------------------------------------------------------------
class ZephyrVirtualMachine
{
public:
	ZephyrVirtualMachine();

	void Startup();
	void Shutdown();

	void InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, ZephyrValueMap* globalVariables, Entity* parentEntity = nullptr, EventArgs* eventArgs = nullptr );

private:
	void		AddEventArgsToLocalVariables( EventArgs* eventArgs, ZephyrValueMap& localVariables );

	void		PushConstant( const ZephyrValue& number );
	ZephyrValue PopConstant();
	ZephyrValue PeekConstant();

	void PushBinaryOp( const ZephyrValue& a, const ZephyrValue& b, eOpCode opCode );
	void PushNumberBinaryOp( NUMBER_TYPE a, NUMBER_TYPE b, eOpCode opCode );
	void PushStringBinaryOp( const std::string& a, const std::string& b, eOpCode opCode );

	void UpdateGlobalVariables( ZephyrValueMap& globalVariables, const ZephyrValueMap& localVariables );
	void ClearConstantStack();

private:
	std::stack<ZephyrValue> m_constantStack;
};

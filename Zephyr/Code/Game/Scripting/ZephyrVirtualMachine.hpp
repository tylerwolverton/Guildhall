#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <stack>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class Entity;


//-----------------------------------------------------------------------------------------------
class ZephyrVirtualMachine
{
	friend class ZephyrInterpreter;

private:
	ZephyrVirtualMachine();
	
	void InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
									  ZephyrValueMap* globalVariables, 
									  Entity* parentEntity = nullptr );

	void InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
									  ZephyrValueMap* globalVariables, 
									  Entity* parentEntity = nullptr, 
									  EventArgs* eventArgs = nullptr, 
									  ZephyrValueMap* stateVariables = nullptr );

	void		InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
										ZephyrValueMap* globalVariables, 
										Entity* parentEntity = nullptr, 
										EventArgs* eventArgs = nullptr, 
										ZephyrValueMap* stateVariables = nullptr );

	void		CopyEventArgVariables( EventArgs* eventArgs );

	void		PushConstant( const ZephyrValue& number );
	ZephyrValue PopConstant();
	ZephyrValue PeekConstant();

	void PushBinaryOp( const ZephyrValue& a, const ZephyrValue& b, eOpCode opCode );
	void PushNumberBinaryOp( NUMBER_TYPE a, NUMBER_TYPE b, eOpCode opCode );
	void PushVec2BinaryOp( const Vec2& a, const Vec2& b, eOpCode opCode );
	void PushStringBinaryOp( const std::string& a, const std::string& b, eOpCode opCode );

	ZephyrValue GetVariableValue( const std::string& variableName, const ZephyrValueMap& localVariables );
	void		AssignToVariable( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	void		AssignToMemberVariable( const std::string& variableName, const std::string& memberName, const ZephyrValue& value, ZephyrValueMap& localVariables );

	void ClearConstantStack();
	void ResetVariableMaps();

private:
	std::stack<ZephyrValue> m_constantStack;

	ZephyrValueMap* m_globalVariables;
	ZephyrValueMap* m_stateVariables;
	ZephyrValueMap m_eventsVariablesCopy;
};

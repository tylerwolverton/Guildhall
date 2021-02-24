#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <stack>
#include <queue>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class Entity;


constexpr int ERROR_ZEPHYR_VAL = -1000;


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
	void PushBoolBinaryOp( bool a, bool b, eOpCode opCode );
	void PushStringBinaryOp( const std::string& a, const std::string& b, eOpCode opCode );

	ZephyrValue GetVariableValue( const std::string& variableName, const ZephyrValueMap& localVariables );
	void		AssignToVariable( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	void		AssignToMemberVariable( const std::string& variableName, const std::string& memberName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	
	void ReportError( const std::string& errorMsg );
	bool IsErrorValue( const ZephyrValue& zephyrValue );

private:
	Entity* m_parentEntity = nullptr;
	std::stack<ZephyrValue> m_constantStack;
	std::deque<std::string> m_curMemberAccessorNames;

	ZephyrValueMap* m_globalVariables;
	ZephyrValueMap* m_stateVariables;
	ZephyrValueMap m_eventsVariablesCopy;
};

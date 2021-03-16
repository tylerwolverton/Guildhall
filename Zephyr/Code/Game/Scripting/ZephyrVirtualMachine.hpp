#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <stack>
#include <queue>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class Entity;


//-----------------------------------------------------------------------------------------------
struct MemberAccessorResult
{
public:
	ZephyrValue finalMemberVal = ZephyrValue( ERROR_ZEPHYR_VAL );
	std::string	baseObjName;
	std::vector<std::string> memberNames;
	std::vector<EntityId> entityIdChain;
};


//-----------------------------------------------------------------------------------------------
class ZephyrVirtualMachine
{
	friend class ZephyrInterpreter;

private:
	ZephyrVirtualMachine();
	
	void		InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
										ZephyrValueMap* globalVariables, 
										Entity* parentEntity = nullptr, 
										EventArgs* eventArgs = nullptr, 
										ZephyrValueMap* stateVariables = nullptr );

	void		CopyEventArgVariables( EventArgs* eventArgs, ZephyrValueMap& localVariables );

	void		PushConstant( const ZephyrValue& number );
	ZephyrValue PopConstant();
	ZephyrValue PeekConstant();

	void PushBinaryOp( ZephyrValue& a, ZephyrValue& b, eOpCode opCode );
	void PushAddOp( ZephyrValue& a, ZephyrValue& b );
	void PushSubtractOp( ZephyrValue& a, ZephyrValue& b );
	void PushMultiplyOp( ZephyrValue& a, ZephyrValue& b );
	void PushDivideOp( ZephyrValue& a, ZephyrValue& b );
	void PushNotEqualOp( ZephyrValue& a, ZephyrValue& b );
	void PushEqualOp( ZephyrValue& a, ZephyrValue& b );
	void PushGreaterOp( ZephyrValue& a, ZephyrValue& b );
	void PushGreaterEqualOp( ZephyrValue& a, ZephyrValue& b );
	void PushLessOp( ZephyrValue& a, ZephyrValue& b );
	void PushLessEqualOp( ZephyrValue& a, ZephyrValue& b );

	ZephyrValue GetVariableValue( const std::string& variableName, const ZephyrValueMap& localVariables );
	void		AssignToVariable( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	void		AssignToVec2MemberVariable( const std::string& variableName, const std::string& memberName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	
	MemberAccessorResult ProcessResultOfMemberAccessor( const ZephyrValueMap& localVariables );
	
	std::map<std::string, std::string> GetCallerVariableToParamNamesFromParameters( const std::string& eventName );
	void InsertParametersIntoEventArgs( EventArgs& args );
	void UpdateIdentifierParameters( const std::map<std::string, std::string>& identifierParams, const EventArgs& args, ZephyrValueMap& localVariables );
	ZephyrValue GetZephyrValFromEventArgs( const std::string& varName, const EventArgs& args );

	ZephyrValue GetGlobalVariableFromEntity	( EntityId entityId, const std::string& variableName );
	void SetGlobalVariableInEntity			( EntityId entityId, const std::string& variableName, const ZephyrValue& value );
	void SetGlobalVec2MemberVariableInEntity( EntityId entityId, const std::string& variableName, const std::string& memberName, const ZephyrValue& value );
	void CallMemberFunctionOnEntity			( EntityId entityId, const std::string& functionName, EventArgs* args );


	void ReportError( const std::string& errorMsg );
	bool IsErrorValue( const ZephyrValue& zephyrValue );

private:
	Entity* m_parentEntity = nullptr;
	std::stack<ZephyrValue> m_constantStack;
	std::deque<std::string> m_curMemberAccessorNames;

	ZephyrValueMap* m_globalVariables;
	ZephyrValueMap* m_stateVariables;
	ZephyrValueMap m_eventVariablesCopy;
	ZephyrValueMap* m_eventVariables;
};

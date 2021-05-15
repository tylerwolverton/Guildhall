#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrInterpreter
{
public:
	static void InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk,
											 ZephyrValueMap* globalVariables,
											 Entity* parentEntity = nullptr,
											 ZephyrValueMap* stateVariables = nullptr );

	static void InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk,
											 ZephyrValueMap* globalVariables,
											 Entity* parentEntity = nullptr,
											 EventArgs* eventArgs = nullptr,
											 ZephyrValueMap* stateVariables = nullptr );
};

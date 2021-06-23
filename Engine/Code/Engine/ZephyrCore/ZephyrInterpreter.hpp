#pragma once
#include "Engine/ZephyrCore/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrEntity;
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrInterpreter
{
public:
	static void BeginFrame();
	static void EndFrame();

	static void InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk,
											 ZephyrValueMap* globalVariables,
											 ZephyrEntity* parentEntity = nullptr,
											 ZephyrValueMap* stateVariables = nullptr );

	static void InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk,
											 ZephyrValueMap* globalVariables,
											 ZephyrEntity* parentEntity = nullptr,
											 EventArgs* eventArgs = nullptr,
											 ZephyrValueMap* stateVariables = nullptr );
};

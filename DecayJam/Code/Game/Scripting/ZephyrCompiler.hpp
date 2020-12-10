#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrCompiler
{
public:
	static ZephyrScriptDefinition* CompileScriptFile( const std::string& filePath );
};

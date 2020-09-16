#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;


//-----------------------------------------------------------------------------------------------
class ZephyrCompiler
{
public:
	static std::vector<ZephyrBytecodeChunk*> CompileScriptFile( const std::string& filePath );
};

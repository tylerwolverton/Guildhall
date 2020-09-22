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
	static bool CompileScriptFile( const std::string& filePath, std::vector<ZephyrBytecodeChunk*>& out_bytecodeChunks );
};

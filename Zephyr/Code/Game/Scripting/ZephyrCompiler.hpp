#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrToken;


//-----------------------------------------------------------------------------------------------
class ZephyrCompiler
{
public:
	static void CompileScriptFile( const std::string& filePath );

private:
	static std::vector<ZephyrToken> ScanSourceIntoTokens( const std::string& scriptSource );
};

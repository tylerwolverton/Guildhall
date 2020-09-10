#include "Game/Scripting/ZephyrCompiler.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/Scripting/ZephyrToken.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrCompiler::CompileScriptFile( const std::string& filePath )
{
	std::string scriptSource( (char*)FileReadToNewBuffer( filePath ) );

	std::vector<ZephyrToken> tokens = ScanSourceIntoTokens( scriptSource );

	g_devConsole->PrintString( Stringf( "Read in:\n %s", scriptSource.c_str() ) );
}


//-----------------------------------------------------------------------------------------------
std::vector<ZephyrToken> ZephyrCompiler::ScanSourceIntoTokens( const std::string& scriptSource )
{
	std::vector<ZephyrToken> tokens;

	return tokens;
}


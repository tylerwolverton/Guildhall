#include "Engine/ZephyrCore/ZephyrCompiler.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/ZephyrCore/ZephyrToken.hpp"
#include "Engine/ZephyrCore/ZephyrBytecodeChunk.hpp"
#include "Engine/ZephyrCore/ZephyrScanner.hpp"
#include "Engine/ZephyrCore/ZephyrParser.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition* ZephyrCompiler::CompileScriptFile( const std::string& filePath )
{
	std::string scriptSource( (char*)FileReadToNewBuffer( filePath ) );

	ZephyrScanner scanner( scriptSource );
	std::vector<ZephyrToken> tokens = scanner.ScanSourceIntoTokens();

	//for ( int tokenIdx = 0; tokenIdx < (int)tokens.size(); ++tokenIdx )
	//{
	//	g_devConsole->PrintString( Stringf( "%s line: %i - %s", tokens[tokenIdx].GetDebugName().c_str(), tokens[tokenIdx].GetLineNum(), tokens[tokenIdx].GetData().c_str() ) );
	//}

	ZephyrParser parser( GetFileName( filePath ), tokens );
	return parser.ParseTokensIntoScriptDefinition();
}


#include "Game/Scripting/ZephyrCompiler.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/Scripting/ZephyrToken.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Game/Scripting/ZephyrScanner.hpp"
#include "Game/Scripting/ZephyrParser.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrCompiler::CompileScriptFile( const std::string& filePath )
{
	std::string scriptSource( (char*)FileReadToNewBuffer( filePath ) );

	ZephyrScanner scanner( scriptSource );
	std::vector<ZephyrToken> tokens = scanner.ScanSourceIntoTokens();

	for ( int tokenIdx = 0; tokenIdx < (int)tokens.size(); ++tokenIdx )
	{
		g_devConsole->PrintString( Stringf( "%s line: %i - %s", tokens[tokenIdx].GetDebugName().c_str(), tokens[tokenIdx].GetLineNum(), tokens[tokenIdx].GetData().c_str() ) );
	}

	ZephyrParser parser( tokens );
	std::vector<ZephyrBytecodeChunk*> bytecodeChunks = parser.ParseTokensIntoBytecodeChunks();

	// TODO: Ownership of this will eventually be transferred to ZephyrScriptDefinition
	PTR_VECTOR_SAFE_DELETE( bytecodeChunks );
}

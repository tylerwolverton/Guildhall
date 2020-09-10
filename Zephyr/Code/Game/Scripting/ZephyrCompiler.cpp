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

	for ( int tokenIdx = 0; tokenIdx < (int)tokens.size(); ++tokenIdx )
	{
		g_devConsole->PrintString( Stringf( "%s line: %i", tokens[tokenIdx].GetDebugName().c_str(), tokens[tokenIdx].GetLineNum() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ParseAndAddToTokenList( std::vector<ZephyrToken>& tokens, std::string& word, int lineNum )
{
	if ( word.empty() )
	{
		return;
	}

	ZephyrToken newToken;
	if ( word == "StateMachine" )	{ newToken = ZephyrToken( eTokenType::STATE_MACHINE, lineNum ); }
	else if ( word == "State" )		{ newToken = ZephyrToken( eTokenType::STATE, lineNum ); }
	else if ( word == "Number" )	{ newToken = ZephyrToken( eTokenType::NUMBER, lineNum ); }
	else							{ newToken = ZephyrToken( eTokenType::UNKNOWN, lineNum ); }

	tokens.push_back( newToken );
	word.clear();
}


//-----------------------------------------------------------------------------------------------
std::vector<ZephyrToken> ZephyrCompiler::ScanSourceIntoTokens( const std::string& scriptSource )
{
	std::vector<ZephyrToken> tokens;

	int curLineNum = 1;
	std::string curWord;

	for ( int srcIdx = 0; srcIdx < (int)scriptSource.size(); ++srcIdx )
	{
		switch ( scriptSource[srcIdx] )
		{
			// Skip whitespace and comments (once we have comments...)
			case ' ':
			case '\t':
			case '\r':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				continue;
			}
			break;

			case '\n':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				++curLineNum;
				continue;
			}
			break;

			// Parse text into word
			// Compare against known token types
			case '{':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				tokens.push_back( ZephyrToken( eTokenType::BRACE_LEFT, curLineNum ) );
			}
			break;

			case '}':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				tokens.push_back( ZephyrToken( eTokenType::BRACE_RIGHT, curLineNum ) );
			}
			break;

			case '+':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				tokens.push_back( ZephyrToken( eTokenType::PLUS, curLineNum ) );
			}
			break;

			case '-':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				tokens.push_back( ZephyrToken( eTokenType::MINUS, curLineNum ) );
			}
			break;

			case '*':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				tokens.push_back( ZephyrToken( eTokenType::STAR, curLineNum ) );
			}
			break;

			case '/':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				tokens.push_back( ZephyrToken( eTokenType::SLASH, curLineNum ) );
			}
			break;

			case ';':
			{
				ParseAndAddToTokenList( tokens, curWord, curLineNum );
				tokens.push_back( ZephyrToken( eTokenType::SEMICOLON, curLineNum ) );
			}
			break;

			default:
			{
				curWord += scriptSource[srcIdx];
			}
			break;
		}
	}

	ParseAndAddToTokenList( tokens, curWord, curLineNum );
	tokens.push_back( ZephyrToken( eTokenType::END_OF_FILE, curLineNum ) );

	return tokens;
}


#include "Game/Scripting/ZephyrParser.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Game/Scripting/ZephyrToken.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrParser::ZephyrParser( const std::vector<ZephyrToken>& tokens )
	: m_tokens( tokens )
{
}


//-----------------------------------------------------------------------------------------------
std::vector<ZephyrBytecodeChunk> ZephyrParser::ParseTokensIntoBytecodeChunks()
{
	// First token must be StateMachine
	ZephyrToken nextToken = ConsumeNextToken();
	if ( nextToken.GetType() != eTokenType::STATE_MACHINE )
	{
		ReportError( "File must begin with a StateMachine definition" );
		return m_bytecodeChunks;
	}

	if ( !ParseBlock() )
	{
		return m_bytecodeChunks;
	}

	nextToken = ConsumeNextToken();
	if ( !DoesTokenMatchType( nextToken, eTokenType::END_OF_FILE ) )
	{
		ReportError( "Nothing can be defined outside StateMachine" );
		return m_bytecodeChunks;
	}

	m_isErrorFree = true;
	return m_bytecodeChunks;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBlock()
{
	// Parse first brace
	ZephyrToken curToken = ConsumeNextToken();
	if ( !DoesTokenMatchType( curToken, eTokenType::BRACE_LEFT ) )
	{
		ReportError( "Expected '{'" );
		return false;
	}

	// Parse statements in block
	while ( !DoesTokenMatchType( curToken, eTokenType::BRACE_RIGHT )
			&& !DoesTokenMatchType( curToken, eTokenType::END_OF_FILE ) )
	{
		if ( !ParseStatement() )
		{
			return false;
		}

		curToken = ConsumeNextToken();
	}

	// Check for closing brace
	if ( !DoesTokenMatchType( curToken, eTokenType::BRACE_RIGHT ) )
	{
		ReportError( "Expected '}'" );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseStatement()
{
	ZephyrToken curToken = ConsumeNextToken();

	switch ( curToken.GetType() )
	{
		// Do something fancier with state name later
		case eTokenType::STATE:		return ParseBlock();
		case eTokenType::NUMBER:	return ParseNumberDeclaration();

		default:
		{
			std::string errorMsg( "Unknown '" );
			errorMsg += curToken.GetDebugName();
			errorMsg += "' seen";

			ReportError( errorMsg );
			return false;
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseNumberDeclaration()
{
	ZephyrToken curToken = ConsumeNextToken();
	if ( !DoesTokenMatchType( curToken, eTokenType::IDENTIFIER ) )
	{
		ReportError( "Expected variable name after 'Number'" );
		return false;
	}

	curToken = ConsumeNextToken();
	switch( curToken.GetType() )
	{
		// TODO: Save variable in table
		case eTokenType::SEMICOLON: break;
		case eTokenType::EQUAL:		return ParseNumberExpression();

		default:
		{
			std::string errorMsg( "Unexpected '" );
			errorMsg += curToken.GetDebugName();
			errorMsg += "' seen, expected ';' or '='";

			ReportError( errorMsg );
			return false;
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseNumberExpression()
{
	ConsumeNextToken();

	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::ReportError( const std::string& errorMsg )
{
	ZephyrToken const& curToken = GetLastToken();

	std::string fullErrorStr = Stringf( "Error - %s: line %i: %s", "filename", curToken.GetLineNum(), errorMsg.c_str() );

	g_devConsole->PrintError( fullErrorStr );
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::ConsumeNextToken()
{
	if ( IsAtEnd() )
	{
		return m_tokens[m_tokens.size() - 1] ;
	}

	return m_tokens[m_curTokenIdx++];
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::GetLastToken()
{
	return m_tokens[m_curTokenIdx - 1];
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::IsCurTokenType( const eTokenType& type )
{
	if ( IsAtEnd()
		 || GetCurTokenType() != type )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::DoesTokenMatchType( const ZephyrToken& token, const eTokenType& type )
{
	if ( token.GetType() != type )
	{
		return false;
	}

	return true;

}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::IsAtEnd()
{
	return m_curTokenIdx >= (int)m_tokens.size();
}


//-----------------------------------------------------------------------------------------------
eTokenType ZephyrParser::GetCurTokenType()
{
	return m_tokens[m_curTokenIdx].GetType();
}


//-----------------------------------------------------------------------------------------------
int ZephyrParser::GetCurTokenLineNum()
{
	return m_tokens[m_curTokenIdx].GetLineNum();
}



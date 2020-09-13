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
	if ( m_tokens[m_curTokenIdx].GetType() != eTokenType::STATE_MACHINE )
	{
		ReportError( "File must begin with a StateMachine definition" );
		return m_bytecodeChunks;
	}

	if ( !ParseBlock() )
	{
		return m_bytecodeChunks;
	}

	m_isErrorFree = true;
	return m_bytecodeChunks;
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
bool ZephyrParser::ParseBlock()
{
	// Parse first brace
	AdvanceToNextToken();
	if ( !IsCurTokenType( eTokenType::BRACE_LEFT ) )
	{
		ReportError( "Expected '{'" );
		return false;
	}

	// Parse statements in block
	AdvanceToNextToken();
	while ( !IsCurTokenType( eTokenType::BRACE_RIGHT ) )
	{
		if ( !ParseStatement() )
		{
			return false;
		}
	}

	// Check for closing brace
	if ( !IsCurTokenType( eTokenType::BRACE_RIGHT ) )
	{
		ReportError( "Expected '}'" );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseStatement()
{
	AdvanceToNextToken();

	switch ( GetCurTokenType() )
	{
		// Do something fancier with state name later
		case eTokenType::STATE:		return ParseBlock();
		case eTokenType::NUMBER:	return ParseNumberDeclaration();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseNumberDeclaration()
{
	AdvanceToNextToken();
	if ( !IsCurTokenType( eTokenType::IDENTIFIER ) )
	{
		ReportError( "Expected variable name after 'Number'" );
		return false;
	}

	AdvanceToNextToken();
	switch( GetCurTokenType() )
	{
		// TODO: Save variable in table
		case eTokenType::SEMICOLON: break;
		case eTokenType::EQUAL:		return ParseNumberExpression();

		default:
		{
			std::string errorMsg( "Unexpected '" );
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
	AdvanceToNextToken();

	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::ReportError( const std::string& errorMsg )
{
	ZephyrToken& curToken = m_tokens[m_curTokenIdx];

	std::string fullErrorStr = Stringf( "Error - %s: line %i: %s", "filename", curToken.GetLineNum(), errorMsg.c_str() );

	g_devConsole->PrintError( fullErrorStr );
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::AdvanceToNextToken()
{
	++m_curTokenIdx;
	return m_tokens[m_curTokenIdx - 1];
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



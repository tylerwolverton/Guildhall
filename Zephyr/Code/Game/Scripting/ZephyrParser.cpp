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
std::vector<ZephyrBytecodeChunk*> ZephyrParser::ParseTokensIntoBytecodeChunks()
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
bool ZephyrParser::CreateBytecodeChunk( const std::string& chunkName )
{
	if ( m_curBytecodeChunk != nullptr )
	{
		ReportError( "Tried to define a new bytecode chunk while writing to existing one, make Tyler fix this" );
		return false;
	}

	ZephyrBytecodeChunk* newChunk = new ZephyrBytecodeChunk( chunkName );
	m_bytecodeChunks.push_back( newChunk );

	m_curBytecodeChunk = newChunk;

	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::FinalizeCurBytecodeChunk()
{
	m_curBytecodeChunk = nullptr;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::WriteByteToCurChunk( byte newByte )
{
	if ( m_curBytecodeChunk == nullptr )
	{
		ReportError( "No active bytecode chunks to write data to" );
		return false;
	}

	m_curBytecodeChunk->WriteByte( newByte );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::WriteOpCodeToCurChunk( eOpCode opCode )
{
	if ( m_curBytecodeChunk == nullptr )
	{
		ReportError( "No active bytecode chunks to write data to" );
		return false;
	}

	m_curBytecodeChunk->WriteByte( opCode );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::WriteNumberConstantToCurChunk( NUMBER_TYPE numConstant )
{
	if ( m_curBytecodeChunk == nullptr )
	{
		ReportError( "No active bytecode chunks to write data to" );
		return false;
	}

	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT_NUMBER );
	m_curBytecodeChunk->WriteNumberConstant( numConstant );

	return true;
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
		case eTokenType::STATE:				
		{
			bool succeeded = CreateBytecodeChunk( "test" );
			if ( !succeeded )
			{
				return false;
			}

			succeeded = ParseBlock();

			FinalizeCurBytecodeChunk();

			return succeeded;
		}
		break;

		case eTokenType::NUMBER:			return ParseNumberDeclaration();

		// TEMP for testing
		case eTokenType::CONSTANT_NUMBER:	
		{
			NUMBER_TYPE out_value = 0.f;
			bool succeeded = ParseNumberExpression( out_value );



			return succeeded;
		}

		default:
		{
			std::string errorMsg( "Unknown statement '" );
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
	ZephyrToken identifier = ConsumeNextToken();
	if ( !DoesTokenMatchType( identifier, eTokenType::IDENTIFIER ) )
	{
		ReportError( "Expected variable name after 'Number'" );
		return false;
	}
	
	// TODO: Update for variant types
	m_curBytecodeChunk->SetVariable( identifier.GetData(), 0.f );

	ZephyrToken curToken = ConsumeNextToken();
	switch( curToken.GetType() )
	{
		// TODO: Save variable in table
		case eTokenType::SEMICOLON: break;
		case eTokenType::EQUAL:		
		{
			// Write an assignment byte

			if ( !ParseExpression() )
			{
				return false;
			}

			return ConsumeExpectedNextToken( eTokenType::SEMICOLON );
		}

		default:
		{
			std::string errorMsg( "Unexpected '" );
			errorMsg += curToken.GetDebugName();
			errorMsg += "' seen, expected ';' or '=' after Number declaration";

			ReportError( errorMsg );
			return false;
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseExpression()
{
	return ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel::ASSIGNMENT );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel precLevel )
{
	ZephyrToken curToken = GetCurToken();
	if ( !CallPrefixFunction( curToken ) )
	{
		// TODO: Make this more descriptive
		ReportError( "Missing expression" );
		return false;
	}

	curToken = GetCurToken();
	while ( precLevel <= GetPrecedenceLevel( curToken ) )
	{
		CallInfixFunction( curToken );
		curToken = GetCurToken();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseParenthesesGroup()
{
	ConsumeNextToken();

	if ( !ParseExpression() )
	{
		return false;
	}
	
	return ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseUnaryExpression()
{
	ZephyrToken curToken = ConsumeNextToken();

	if ( !ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel::UNARY ) )
	{
		return false;
	}

	switch ( curToken.GetType() )
	{
		case eTokenType::MINUS:
		{
			return WriteOpCodeToCurChunk( eOpCode::NEGATE );
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBinaryExpression()
{
	ZephyrToken curToken = ConsumeNextToken();

	eOpPrecedenceLevel precLevel = GetNextHighestPrecedenceLevel( curToken );

	if ( !ParseExpressionWithPrecedenceLevel( precLevel ) )
	{
		return false;
	}

	switch ( curToken.GetType() )
	{
		case eTokenType::PLUS:	return WriteOpCodeToCurChunk( eOpCode::ADD );
		case eTokenType::MINUS:	return WriteOpCodeToCurChunk( eOpCode::SUBTRACT );
		case eTokenType::STAR:	return WriteOpCodeToCurChunk( eOpCode::MULTIPLY );
		case eTokenType::SLASH:	return WriteOpCodeToCurChunk( eOpCode::DIVIDE );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseNumberExpression( NUMBER_TYPE& out_result )
{
	ZephyrToken curToken = ConsumeNextToken();

	out_result = (NUMBER_TYPE)atof( curToken.GetData().c_str() );
	return WriteNumberConstantToCurChunk( out_result );
	//while ( curToken.GetType() != eTokenType::SEMICOLON )
	//{
	//	switch ( curToken.GetType() )
	//	{
	//		// Token is just a number, return it
	//		case eTokenType::CONSTANT_NUMBER:
	//		{
	//			out_result = (NUMBER_TYPE)atof( curToken.GetData().c_str() );
	//			return WriteNumberConstantToCurChunk( out_result );
	//		}
	//		break;

	//		case eTokenType::PLUS:
	//		{
	//			NUMBER_TYPE rightResult;
	//			ParseNumberExpression( rightResult );

	//			WriteNumberConstantToCurChunk( rightResult );

	//			WriteOpCodeToCurChunk( eOpCode::ADD );
	//			return true;
	//		}
	//		break;
	//	}
	//}

	//return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CallPrefixFunction( const ZephyrToken& token )
{
	NUMBER_TYPE out;
	switch ( token.GetType() )
	{
		case eTokenType::PARENTHESIS_LEFT:	return ParseParenthesesGroup();
		case eTokenType::CONSTANT_NUMBER:	return ParseNumberExpression( out );
		case eTokenType::MINUS:				return ParseUnaryExpression();
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CallInfixFunction( const ZephyrToken& token )
{
	switch ( token.GetType() )
	{
		case eTokenType::PLUS:	
		case eTokenType::MINUS:	
		case eTokenType::STAR:	
		case eTokenType::SLASH:	
		{
			return ParseBinaryExpression();
		}
		break;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
eOpPrecedenceLevel ZephyrParser::GetPrecedenceLevel( const ZephyrToken& token )
{
	switch ( token.GetType() )
	{
		case eTokenType::PLUS:		return eOpPrecedenceLevel::TERM;
		case eTokenType::MINUS:		return eOpPrecedenceLevel::TERM;
		case eTokenType::STAR:		return eOpPrecedenceLevel::FACTOR;
		case eTokenType::SLASH:		return eOpPrecedenceLevel::FACTOR;
		default:					return eOpPrecedenceLevel::NONE;
	}
}


//-----------------------------------------------------------------------------------------------
eOpPrecedenceLevel ZephyrParser::GetNextHighestPrecedenceLevel( const ZephyrToken& token )
{
	eOpPrecedenceLevel precLevel = GetPrecedenceLevel( token );

	//TODO: MAke this safer
	return (eOpPrecedenceLevel)( (int)precLevel + 1 );
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
bool ZephyrParser::ConsumeExpectedNextToken( eTokenType expectedType )
{
	ZephyrToken curToken = ConsumeNextToken();

	if ( !DoesTokenMatchType( curToken, expectedType ) )
	{
		ReportError( Stringf( "Expected '%s'", ToString( expectedType ).c_str() ) );
		return false;
	}

	return true;
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
ZephyrToken ZephyrParser::GetCurToken()
{
	return m_tokens[m_curTokenIdx];
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



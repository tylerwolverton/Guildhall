#include "Game/Scripting/ZephyrParser.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Game/Scripting/ZephyrToken.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrParser::ZephyrParser( const std::string& filename, const std::vector<ZephyrToken>& tokens )
	: m_filename( filename )
	, m_tokens( tokens )
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
bool ZephyrParser::WriteConstantToCurChunk( const ZephyrValue& constant )
{
	if ( m_curBytecodeChunk == nullptr )
	{
		ReportError( "No active bytecode chunks to write data to" );
		return false;
	}

	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT );
	m_curBytecodeChunk->WriteConstant( constant );

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

		curToken = GetCurToken();
	}

	// Check for closing brace
	curToken = ConsumeNextToken();
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
		case eTokenType::STATE:				
		{
			curToken = ConsumeNextToken();
			if ( curToken.GetType() != eTokenType::IDENTIFIER )
			{
				ReportError( "State must be followed by a name" );
				return false;
			}

			bool succeeded = CreateBytecodeChunk( curToken.GetData() );
			if ( !succeeded )
			{
				return false;
			}

			succeeded = ParseBlock();

			FinalizeCurBytecodeChunk();

			return succeeded;
		}
		break;

		case eTokenType::NUMBER:			
		{
			if ( !ParseNumberDeclaration() )
			{
				return false;
			}
		}
		break;

		case eTokenType::FIRE_EVENT:
		{
			if ( !ParseFireEvent() )
			{
				return false;
			}
		}
		break;

		case eTokenType::IDENTIFIER:
		{
			if ( !ParseAssignment() )
			{
				return false;
			}
		}
		break;

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

	if ( !ConsumeExpectedNextToken( eTokenType::SEMICOLON ) )
	{
		return false;
	}

	return true;
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
	
	m_curBytecodeChunk->SetVariable( identifier.GetData(), ZephyrValue( 0.f ) );

	ZephyrToken curToken = GetCurToken();
	switch( curToken.GetType() )
	{
		case eTokenType::SEMICOLON: 
		{
			WriteConstantToCurChunk( ZephyrValue( 0.f ) );
		}
		break;
		
		case eTokenType::EQUAL:		
		{
			AdvanceToNextToken();

			// Write an assignment byte
			if ( !ParseExpression() )
			{
				return false;
			}

			/*if ( !ConsumeExpectedNextToken( eTokenType::SEMICOLON ) )
			{
				return false;
			}*/
		}
		break;

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

	WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::DEFINE_VARIABLE );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseFireEvent()
{
	// FireEvent opening paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	ZephyrToken eventName = ConsumeNextToken();
	if ( !DoesTokenMatchType( eventName, eTokenType::IDENTIFIER ) )
	{
		ReportError( "FireEvent must specify an event to call in parentheses" );
		return false;
	}

	// Event opening paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	if ( !ParseEventArgs() )
	{
		return false;
	}

	// Event closing paren
	if ( GetCurToken().GetType() != eTokenType::PARENTHESIS_RIGHT )
	{
		ReportError( "Expected ')' after parameter list for event" );
		return false;
	}

	// FireEvent closing paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) )
	{
		return false;
	}

	WriteConstantToCurChunk( ZephyrValue( eventName.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::FIRE_EVENT );
	
	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseEventArgs()
{
	ZephyrToken identifier = ConsumeNextToken();
	int paramCount = 0;

	while ( identifier.GetType() == eTokenType::IDENTIFIER )
	{
		
		if ( !ConsumeExpectedNextToken( eTokenType::EQUAL ) )
		{
			ReportError( "Parameter to event must be in the form, var = value" );
			return false;
		}

		ZephyrToken valueToken = ConsumeNextToken();
		switch ( valueToken.GetType() )
		{
			case eTokenType::CONSTANT_NUMBER:
			{
				WriteConstantToCurChunk( valueToken.GetData() );
			}
			break;

			case eTokenType::IDENTIFIER:
			{
				ZephyrValue value;
				if ( !m_curBytecodeChunk->TryToGetVariable( valueToken.GetData(), value ) )
				{
					ReportError( Stringf( "Undefined variable, '%s', cannot be used as a parameter ", valueToken.GetData().c_str() ) );
					return false;
				}

				WriteConstantToCurChunk( value );
				//WriteOpCodeToCurChunk( eOpCode::GET_VARIABLE_VALUE );
			}
			break;

			default:
			{
				ReportError( "Must set parameter equal to a value in the form, var = value" );
				return false;
			}
		}

		WriteConstantToCurChunk( identifier.GetData() );
		++paramCount;

		AdvanceToNextTokenIfTypeMatches( eTokenType::COMMA );

		identifier = ConsumeNextToken();
	}

	WriteConstantToCurChunk( ZephyrValue( (float)paramCount ) );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseAssignment()
{
	ZephyrToken identifier = GetLastToken();
	ZephyrToken curToken = ConsumeNextToken();

	if ( curToken.GetType() != eTokenType::EQUAL )
	{
		ReportError( Stringf( "Assignment to variable '%s' expected a '=' sign after the variable name", identifier.GetData().c_str() ) );
		return false;
	}

	ZephyrValue value;
	if ( !m_curBytecodeChunk->TryToGetVariable( identifier.GetData(), value ) )
	{
		ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
		return false;
	}

	if ( !ParseExpression() )
	{
		return false;
	}

	WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT );

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
bool ZephyrParser::ParseNumberExpression()
{
	ZephyrToken curToken = ConsumeNextToken();

	return WriteConstantToCurChunk( (NUMBER_TYPE)atof( curToken.GetData().c_str() ) );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseIdentifierExpressionOfType( eValueType expectedType )
{
	ZephyrToken curToken = ConsumeNextToken();

	if ( curToken.GetType() != eTokenType::IDENTIFIER )
	{
		ReportError( Stringf( "Expected Identifier, but found '%s'", ToString( curToken.GetType() ).c_str() ) );
		return false;
	}

	ZephyrValue value;
	if ( !m_curBytecodeChunk->TryToGetVariable( curToken.GetData(), value ) )
	{
		ReportError( Stringf( "Undefined variable seen, '%s'", curToken.GetData().c_str() ) );
		return false;
	}

	if ( value.GetType() != expectedType )
	{
		ReportError( Stringf( "Type mismatch, expected '%s' to be '%s', but instead it was '%s'", curToken.GetData().c_str(), ToString( expectedType ).c_str(), ToString( value.GetType() ).c_str() ) );
		return false;
	}

	WriteConstantToCurChunk( ZephyrValue( curToken.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::GET_VARIABLE_VALUE );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CallPrefixFunction( const ZephyrToken& token )
{
	switch ( token.GetType() )
	{
		case eTokenType::PARENTHESIS_LEFT:	return ParseParenthesesGroup();
		case eTokenType::CONSTANT_NUMBER:	return ParseNumberExpression();
		case eTokenType::MINUS:				return ParseUnaryExpression();
		case eTokenType::IDENTIFIER:		
		{
			if ( PeekNextToken().GetType() == eTokenType::EQUAL )
			{
				AdvanceToNextToken();
				return ParseAssignment();
			}

			return ParseIdentifierExpressionOfType( eValueType::NUMBER );
		}
		break;
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

		/*case eTokenType::EQUAL:
		{
			return ParseAssignment();
		}
		break;*/
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

	std::string fullErrorStr = Stringf( "Error - %s: line %i: %s", m_filename.c_str(), curToken.GetLineNum(), errorMsg.c_str() );

	g_devConsole->PrintError( fullErrorStr );
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::PeekNextToken()
{
	if ( IsAtEnd() )
	{
		return m_tokens[m_tokens.size() - 1];
	}

	return m_tokens[m_curTokenIdx + 1];
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
void ZephyrParser::AdvanceToNextToken()
{
	if ( IsAtEnd() )
	{
		 return;
	}

	m_curTokenIdx++;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::AdvanceToNextTokenIfTypeMatches( eTokenType expectedType )
{
	if ( GetCurToken().GetType() == expectedType )
	{
		AdvanceToNextToken();
	}
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



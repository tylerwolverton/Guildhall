#include "Game/Scripting/ZephyrParser.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Game/Scripting/ZephyrToken.hpp"
#include "Game/Scripting/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrParser::ZephyrParser( const std::string& filename, const std::vector<ZephyrToken>& tokens )
	: m_filename( filename )
	, m_tokens( tokens )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition* ZephyrParser::ParseTokensIntoScriptDefinition()
{
	// First token must be StateMachine
	ZephyrToken nextToken = ConsumeNextToken();
	if ( nextToken.GetType() != eTokenType::STATE_MACHINE )
	{
		ReportError( "File must begin with a StateMachine definition" );
		return nullptr;
	}

	CreateStateMachineBytecodeChunk();

	if ( !ParseBlock() )
	{
		return nullptr;
	}

	nextToken = ConsumeNextToken();
	if ( !DoesTokenMatchType( nextToken, eTokenType::END_OF_FILE ) )
	{
		ReportError( "Nothing can be defined outside StateMachine" );
		return nullptr;
	}

	return new ZephyrScriptDefinition( m_stateMachineBytecodeChunk, m_bytecodeChunks );
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::CreateStateMachineBytecodeChunk()
{
	m_stateMachineBytecodeChunk = new ZephyrBytecodeChunk( "StateMachine" );

	m_curBytecodeChunk = m_stateMachineBytecodeChunk;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CreateBytecodeChunk( const std::string& chunkName )
{
	if ( m_curBytecodeChunk != m_stateMachineBytecodeChunk )
	{
		ReportError( "Tried to define a new bytecode chunk while writing to existing one, make Tyler fix this" );
		return false;
	}

	ZephyrBytecodeChunk* newChunk = new ZephyrBytecodeChunk( chunkName );
	
	for ( auto globalVar : m_stateMachineBytecodeChunk->GetVariables() )
	{
		newChunk->SetVariable( globalVar.first, globalVar.second );
	}

	m_bytecodeChunks[chunkName] = newChunk;

	m_curBytecodeChunk = newChunk;



	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::FinalizeCurBytecodeChunk()
{
	m_curBytecodeChunk = m_stateMachineBytecodeChunk;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::WriteByteToCurChunk( byte newByte )
{
	if ( m_curBytecodeChunk == nullptr )
	{
		ReportError( "No active bytecode chunks to write data to, make Tyler fix this" );
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
		ReportError( "No active bytecode chunks to write data to, make Tyler fix this" );
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
		ReportError( "No active bytecode chunks to write data to, make Tyler fix this" );
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
	if ( !ConsumeExpectedNextToken( eTokenType::BRACE_LEFT ) )
	{
		return false;
	}

	ZephyrToken curToken = GetCurToken();

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
	if ( !ConsumeExpectedNextToken( eTokenType::BRACE_RIGHT ) )
	{
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

			m_curBytecodeChunk->SetType( eBytecodeChunkType::STATE );
			FinalizeCurBytecodeChunk();

			return succeeded;
		}
		break;

		case eTokenType::ON_EVENT:
		{
			if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
			{
				return false;
			}

			curToken = ConsumeNextToken();
			if ( curToken.GetType() != eTokenType::IDENTIFIER )
			{
				ReportError( "OnEvent must specify an event name" );
				return false;
			}

			if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) )
			{
				return false;
			}

			bool succeeded = CreateBytecodeChunk( curToken.GetData() );
			if ( !succeeded )
			{
				return false;
			}

			succeeded = ParseBlock();

			m_curBytecodeChunk->SetType( eBytecodeChunkType::EVENT );
			FinalizeCurBytecodeChunk();

			return succeeded;
		}
		break;

		case eTokenType::IF:
		{
			return ParseIfStatement();
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

		case eTokenType::STRING:
		{
			if ( !ParseStringDeclaration() )
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

		// End of block
		case eTokenType::BRACE_RIGHT:
		{
			return true;
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
	
	DeclareVariable( identifier, eValueType::NUMBER );

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

			if ( !ParseExpression( eValueType::NUMBER ) )
			{
				return false;
			}
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
bool ZephyrParser::ParseStringDeclaration()
{
	ZephyrToken identifier = ConsumeNextToken();
	if ( !DoesTokenMatchType( identifier, eTokenType::IDENTIFIER ) )
	{
		ReportError( "Expected variable name after 'String'" );
		return false;
	}

	DeclareVariable( identifier, eValueType::STRING );

	ZephyrToken curToken = GetCurToken();
	switch ( curToken.GetType() )
	{
		case eTokenType::SEMICOLON:
		{
			WriteConstantToCurChunk( ZephyrValue( std::string( "" ) ) );
		}
		break;

		case eTokenType::EQUAL:
		{
			AdvanceToNextToken();

			if ( !ParseExpression( eValueType::STRING ) )
			//if ( !ParseStringExpression() )
			{
				return false;
			}
		}
		break;

		default:
		{
			std::string errorMsg( "Unexpected '" );
			errorMsg += curToken.GetDebugName();
			errorMsg += "' seen, expected ';' or '=' after String declaration";

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
			case eTokenType::CONSTANT_STRING:
			{
				WriteConstantToCurChunk( valueToken.GetData() );
			}
			break;

			case eTokenType::IDENTIFIER:
			{
				ZephyrValue value;
				if ( !TryToGetVariable( valueToken.GetData(), value ) )
				{
					ReportError( Stringf( "Undefined variable, '%s', cannot be used as a parameter ", valueToken.GetData().c_str() ) );
					return false;
				}

				WriteConstantToCurChunk( ZephyrValue( valueToken.GetData() ) );
				WriteOpCodeToCurChunk( eOpCode::GET_VARIABLE_VALUE );
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
bool ZephyrParser::ParseIfStatement()
{
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) ) return false;

	if ( !ParseExpression( GetNextValueTypeInExpression() ) ) return false;

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) ) return false;

	WriteOpCodeToCurChunk( eOpCode::IF );
	
	// Write a placeholder for how many bytes the if block is so we can update it with the length later
	int instructionCountIdx = m_curBytecodeChunk->AddConstant( ZephyrValue( 0.f ) );
	m_curBytecodeChunk->WriteByte( instructionCountIdx );
	int preBlockbyteCount = m_curTokenIdx;

	if ( !ParseBlock() ) return false;

	m_curBytecodeChunk->SetConstantAtIdx( instructionCountIdx, ZephyrValue( (float)( m_curTokenIdx - preBlockbyteCount ) ) );

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
	if ( !TryToGetVariable( identifier.GetData(), value ) )
	{
		ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
		return false;
	}

	if ( !ParseExpression( value.GetType() ) )
	{
		return false;
	}

	//switch ( value.GetType() )
	//{
	//	case eValueType::NUMBER:
	//	{
	//		if ( !ParseExpression( value.GetType() ) )
	//		{
	//			return false;
	//		}
	//	}
	//	break;

	//	case eValueType::STRING:
	//	{
	//		if ( !ParseStringExpression() )
	//		{
	//			return false;
	//		}
	//	}
	//	break;
	//}

	WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseExpression( const eValueType& expressionType )
{
	return ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel::ASSIGNMENT, expressionType );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel precLevel, const eValueType& expressionType )
{
	ZephyrToken curToken = GetCurToken();
	if ( !CallPrefixFunction( curToken, expressionType ) )
	{
		// TODO: Make this more descriptive
		ReportError( "Missing expression" );
		return false;
	}

	curToken = GetCurToken();
	while ( precLevel <= GetPrecedenceLevel( curToken ) )
	{
		CallInfixFunction( curToken, expressionType );
		curToken = GetCurToken();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseParenthesesGroup( const eValueType& expressionType )
{
	ConsumeNextToken();

	if ( !ParseExpression( expressionType ) )
	{
		return false;
	}
	
	return ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseUnaryExpression( const eValueType& expressionType )
{
	ZephyrToken curToken = ConsumeNextToken();

	if ( !ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel::UNARY, expressionType ) )
	{
		return false;
	}

	switch ( expressionType )
	{
		case eValueType::NUMBER:
		{
			switch ( curToken.GetType() )
			{
				case eTokenType::MINUS:			return WriteOpCodeToCurChunk( eOpCode::NEGATE );
				case eTokenType::BANG:			return WriteOpCodeToCurChunk( eOpCode::NOT );
			}
		}
		break;

		case eValueType::STRING:
		{
			switch ( curToken.GetType() )
			{
				case eTokenType::BANG:			return WriteOpCodeToCurChunk( eOpCode::NOT );
			}
		}
		break;
	}
	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBinaryExpression( const eValueType& expressionType )
{
	ZephyrToken curToken = ConsumeNextToken();

	eOpPrecedenceLevel precLevel = GetNextHighestPrecedenceLevel( curToken );

	if ( !ParseExpressionWithPrecedenceLevel( precLevel, expressionType ) )
	{
		return false;
	}

	switch ( expressionType )
	{
		case eValueType::NUMBER:
		{
			switch ( curToken.GetType() )
			{
				case eTokenType::PLUS:			return WriteOpCodeToCurChunk( eOpCode::ADD );
				case eTokenType::MINUS:			return WriteOpCodeToCurChunk( eOpCode::SUBTRACT );
				case eTokenType::STAR:			return WriteOpCodeToCurChunk( eOpCode::MULTIPLY );
				case eTokenType::SLASH:			return WriteOpCodeToCurChunk( eOpCode::DIVIDE );
				case eTokenType::BANG_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::NOT_EQUAL );
				case eTokenType::EQUAL_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::EQUAL );
				case eTokenType::GREATER:		return WriteOpCodeToCurChunk( eOpCode::GREATER );
				case eTokenType::GREATER_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::GREATER_EQUAL );
				case eTokenType::LESS:			return WriteOpCodeToCurChunk( eOpCode::LESS );
				case eTokenType::LESS_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::LESS_EQUAL );
			}
		}
		break;

		case eValueType::STRING:
		{
			switch ( curToken.GetType() )
			{
				case eTokenType::PLUS:			return WriteOpCodeToCurChunk( eOpCode::ADD );
				case eTokenType::BANG_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::NOT_EQUAL );
				case eTokenType::EQUAL_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::EQUAL );
			}
		}
		break;
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
bool ZephyrParser::ParseStringExpression()
{
	ZephyrToken curToken = ConsumeNextToken();

	return WriteConstantToCurChunk( ZephyrValue( curToken.GetData() ) );

	/*ZephyrToken curToken = GetCurToken();

	switch ( curToken.GetType() )
	{
		case eTokenType::CONSTANT_STRING:
		{
			WriteConstantToCurChunk( ZephyrValue( curToken.GetData() ) );

			AdvanceToNextToken();
		}
		break;

		case eTokenType::IDENTIFIER:
		{
			if ( !ParseIdentifierExpressionOfType( eValueType::STRING ) )
			{
				return false;
			}
		}
		break;

		default:
		{
			ReportError( "Must specify text in quotes or another string in string assignment" );
			return false;
		}
	}

	return true;*/
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
	if ( !TryToGetVariable( curToken.GetData(), value ) )
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
bool ZephyrParser::CallPrefixFunction( const ZephyrToken& token, const eValueType& expectedType )
{
	switch ( token.GetType() )
	{
		case eTokenType::PARENTHESIS_LEFT:	return ParseParenthesesGroup( expectedType );
		case eTokenType::CONSTANT_NUMBER:	return ParseNumberExpression();
		case eTokenType::CONSTANT_STRING:	return ParseStringExpression();
		case eTokenType::MINUS:				return ParseUnaryExpression( expectedType );
		case eTokenType::BANG:				return ParseUnaryExpression( expectedType );
		case eTokenType::IDENTIFIER:		
		{
			if ( PeekNextToken().GetType() == eTokenType::EQUAL )
			{
				AdvanceToNextToken();
				return ParseAssignment();
			}

			return ParseIdentifierExpressionOfType( expectedType );
		}
		break;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CallInfixFunction( const ZephyrToken& token, const eValueType& expectedType )
{
	switch ( token.GetType() )
	{
		case eTokenType::PLUS:	
		case eTokenType::MINUS:	
		case eTokenType::STAR:	
		case eTokenType::SLASH:	
		case eTokenType::BANG_EQUAL:
		case eTokenType::EQUAL_EQUAL:
		case eTokenType::GREATER:
		case eTokenType::GREATER_EQUAL:
		case eTokenType::LESS:
		case eTokenType::LESS_EQUAL:
		{
			return ParseBinaryExpression( expectedType );
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
		case eTokenType::PLUS:				return eOpPrecedenceLevel::TERM;
		case eTokenType::MINUS:				return eOpPrecedenceLevel::TERM;
		case eTokenType::STAR:				return eOpPrecedenceLevel::FACTOR;
		case eTokenType::SLASH:				return eOpPrecedenceLevel::FACTOR;
		case eTokenType::BANG_EQUAL:		return eOpPrecedenceLevel::EQUALITY;
		case eTokenType::EQUAL_EQUAL:		return eOpPrecedenceLevel::EQUALITY;
		case eTokenType::GREATER:			return eOpPrecedenceLevel::COMPARISON;
		case eTokenType::GREATER_EQUAL:		return eOpPrecedenceLevel::COMPARISON;
		case eTokenType::LESS:				return eOpPrecedenceLevel::COMPARISON;
		case eTokenType::LESS_EQUAL:		return eOpPrecedenceLevel::COMPARISON;
		default:							return eOpPrecedenceLevel::NONE;
	}
}


//-----------------------------------------------------------------------------------------------
eOpPrecedenceLevel ZephyrParser::GetNextHighestPrecedenceLevel( const ZephyrToken& token )
{
	eOpPrecedenceLevel precLevel = GetPrecedenceLevel( token );

	//TODO: Make this safer
	return (eOpPrecedenceLevel)( (int)precLevel + 1 );
}


//-----------------------------------------------------------------------------------------------
eValueType ZephyrParser::GetNextValueTypeInExpression()
{
	ZephyrToken curToken = GetCurToken();

	int tokenIdx = m_curTokenIdx;
	while ( curToken.GetType() != eTokenType::END_OF_FILE )
	{
		switch ( curToken.GetType() )
		{
			case eTokenType::CONSTANT_NUMBER: return eValueType::NUMBER;
			case eTokenType::CONSTANT_STRING: return eValueType::STRING;
			case eTokenType::IDENTIFIER:
			{
				ZephyrValue value;
				m_curBytecodeChunk->TryToGetVariable( curToken.GetData(), value );
				return value.GetType();
			}
		}

		++tokenIdx;
		curToken = m_tokens[tokenIdx];
	}

	return eValueType::NONE;
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
void ZephyrParser::DeclareVariable( const ZephyrToken& identifier, const eValueType& varType )
{
	// m_curBytecodeChunk will be the global state machine if outside a state declaration, should never be null
	ZephyrValue value;
	switch ( varType )
	{
		case eValueType::NUMBER: value = ZephyrValue( 0.f ); break;
		case eValueType::STRING: value = ZephyrValue( std::string("") ); break;
		case eValueType::BOOL:	 value = ZephyrValue( false ); break;
	}

	m_curBytecodeChunk->SetVariable( identifier.GetData(), value );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const
{
	if ( !m_curBytecodeChunk->TryToGetVariable( identifier, out_value )
		 && m_curBytecodeChunk != m_stateMachineBytecodeChunk )
	{
		return m_stateMachineBytecodeChunk->TryToGetVariable( identifier, out_value );
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



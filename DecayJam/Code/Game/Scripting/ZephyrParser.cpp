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
		return new ZephyrScriptDefinition( nullptr, m_bytecodeChunks );
	}

	CreateStateMachineBytecodeChunk();

	if ( !ParseBlock() )
	{
		return new ZephyrScriptDefinition( nullptr, m_bytecodeChunks );
	}

	nextToken = ConsumeNextToken();
	if ( !DoesTokenMatchType( nextToken, eTokenType::END_OF_FILE ) )
	{
		ReportError( "Nothing can be defined outside StateMachine" );
		return new ZephyrScriptDefinition( nullptr, m_bytecodeChunks );
	}

	ZephyrScriptDefinition* validScript =  new ZephyrScriptDefinition( m_stateMachineBytecodeChunk, m_bytecodeChunks );
	validScript->SetIsValid( true );

	return validScript;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::CreateStateMachineBytecodeChunk()
{
	m_stateMachineBytecodeChunk = new ZephyrBytecodeChunk( "StateMachine" );
	m_stateMachineBytecodeChunk->SetType( eBytecodeChunkType::STATE_MACHINE );

	m_curBytecodeChunk = m_stateMachineBytecodeChunk;

	m_curBytecodeChunksStack.push( m_stateMachineBytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CreateBytecodeChunk( const std::string& chunkName, const eBytecodeChunkType& type )
{
	if ( type == eBytecodeChunkType::STATE_MACHINE )
	{
		ReportError( "Each file can only have 1 StateMachine definition" );
		return false;
	}

	switch ( m_curBytecodeChunk->GetType() )
	{
		case eBytecodeChunkType::STATE_MACHINE:
		{
			// No error here, can define anything
		}
		break;

		case eBytecodeChunkType::STATE:
		{
			if ( type == eBytecodeChunkType::STATE )
			{
				ReportError( "Cannot define a State inside of another State definition" );
				return false;
			}
		}
		break;

		case eBytecodeChunkType::EVENT:
		{
			switch ( type )
			{
				case eBytecodeChunkType::STATE:
				{
					ReportError( "Cannot define a State inside of an OnEvent definition" );
					return false;
				}
				break;

				case eBytecodeChunkType::EVENT:
				{
					ReportError( "Cannot define an Event inside of another OnEvent definition" );
					return false;
				}
				break;
			}
		}
		break;

		default:
		{
			ReportError( "Tried to define a new bytecode chunk while inside an invalid chunk, make Tyler fix this" );
			return false;
		}
		break;
	}

	// This is a valid chunk definition, create with current chunk as parent scope
	ZephyrBytecodeChunk* newChunk = new ZephyrBytecodeChunk( chunkName, m_curBytecodeChunk );
	
	/*for ( auto globalVar : m_curBytecodeChunk->GetVariables() )
	{
		newChunk->SetVariable( globalVar.first, globalVar.second );
	}*/

	newChunk->SetType( type );

	// Save any state chunks into a map for the ZephyrScriptDefinition
	if ( type == eBytecodeChunkType::STATE )
	{
		// Set the first state seen in the file as the initial state
		if ( m_isFirstStateDef )
		{
			m_isFirstStateDef = false;

			newChunk->SetAsInitialState();
		}

		m_bytecodeChunks[chunkName] = newChunk;
	}

	// Save event chunk into parent chunk's event map
	if ( type == eBytecodeChunkType::EVENT )
	{
		m_curBytecodeChunk->AddEventChunk( newChunk );
	}


	m_curBytecodeChunk = newChunk;
	m_curBytecodeChunksStack.push( newChunk );

	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::FinalizeCurBytecodeChunk()
{
	m_curBytecodeChunksStack.pop();

	if ( !m_curBytecodeChunksStack.empty() )
	{
		m_curBytecodeChunk = m_curBytecodeChunksStack.top();
	}
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

			bool succeeded = CreateBytecodeChunk( curToken.GetData(), eBytecodeChunkType::STATE );
			if ( !succeeded )
			{
				return false;
			}

			succeeded = ParseBlock();

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

			bool succeeded = CreateBytecodeChunk( curToken.GetData(), eBytecodeChunkType::EVENT );
			if ( !succeeded )
			{
				return false;
			}

			succeeded = ParseBlock();

			FinalizeCurBytecodeChunk();

			return succeeded;
		}
		break;

		case eTokenType::ON_ENTER:
		case eTokenType::ON_EXIT:
		{
			if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) ) { return false; }
			if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) ) {	return false; }

			bool succeeded = CreateBytecodeChunk( ToString( curToken.GetType() ), eBytecodeChunkType::EVENT );
			if ( !succeeded )
			{
				return false;
			}

			succeeded = ParseBlock();

			FinalizeCurBytecodeChunk();

			return succeeded;
		}
		break;

		case eTokenType::CHANGE_STATE:
		{
			if ( !ParseChangeStateStatement() )
			{
				return false;
			}
		}
		break;

		case eTokenType::IF:
		{
			return ParseIfStatement();
		}
		break;

		case eTokenType::RETURN:
		{
			m_curBytecodeChunk->WriteByte( eOpCode::RETURN );
		}
		break;

		case eTokenType::NUMBER:			
		{
			if ( !ParseVariableDeclaration( eValueType::NUMBER ) )
			{
				return false;
			}
		}
		break;

		case eTokenType::VEC2:
		{
			if ( !ParseVariableDeclaration( eValueType::VEC2 ) )
			{
				return false;
			}
		}
		break;

		case eTokenType::BOOL:
		{
			if ( !ParseVariableDeclaration( eValueType::BOOL ) )
			{
				return false;
			}
		}
		break;

		case eTokenType::STRING:
		{
			if ( !ParseVariableDeclaration( eValueType::STRING ) )
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
bool ZephyrParser::ParseVariableDeclaration( const eValueType& varType )
{
	ZephyrToken identifier = ConsumeNextToken();
	if ( !DoesTokenMatchType( identifier, eTokenType::IDENTIFIER ) )
	{
		ReportError( Stringf( "Expected variable name after '%s'", ToString( varType ).c_str() ) );
		return false;
	}

	DeclareVariable( identifier, varType );

	ZephyrToken curToken = GetCurToken();
	switch ( curToken.GetType() )
	{
		case eTokenType::SEMICOLON:
		{
			switch ( varType )
			{
				case eValueType::NUMBER: WriteConstantToCurChunk( ZephyrValue( 0.f ) ); break;
				case eValueType::BOOL:	 WriteConstantToCurChunk( ZephyrValue( false ) ); break;
				case eValueType::STRING: WriteConstantToCurChunk( ZephyrValue( "" ) ); break;
				case eValueType::VEC2:	 WriteConstantToCurChunk( ZephyrValue( Vec2::ZERO ) ); break;
			}
		}
		break;

		case eTokenType::EQUAL:
		{
			AdvanceToNextToken();

			if ( !ParseExpression( varType ) )
			{
				return false;
			}

			WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
			switch ( varType )
			{
				case eValueType::NUMBER:
				case eValueType::BOOL:
				case eValueType::STRING:
					WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT ); 
					break;

				case eValueType::VEC2:
					WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT_VEC2 );
					break;
			}
		}
		break;

		default:
		{
			std::string errorMsg( "Unexpected '" );
			errorMsg += curToken.GetDebugName();
			errorMsg += "' seen, expected ';' or '=' after '";
			errorMsg +=	ToString( varType );
			errorMsg +=	" declaration";

			ReportError( errorMsg );
			return false;
		}
		break;
	}

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

		ZephyrToken valueToken = GetCurToken();
		switch ( valueToken.GetType() )
		{
			case eTokenType::CONSTANT_NUMBER:
			{
				if ( !ParseExpression( eValueType::NUMBER ) )
				{
					return false;
				}
			}
			break;

			case eTokenType::VEC2:
			{
				if ( !ParseExpression( eValueType::VEC2 ) )
				{
					return false;
				}
			}
			break;

			case eTokenType::TRUE:
			case eTokenType::FALSE:
			{
				if ( !ParseExpression( eValueType::BOOL ) )
				{
					return false;
				}
			}
			break;

			case eTokenType::CONSTANT_STRING:
			{
				if ( !ParseExpression( eValueType::STRING ) )
				{
					return false;
				}
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

				if ( !ParseExpression( value.GetType() ) )
				{
					return false;
				}
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
bool ZephyrParser::ParseChangeStateStatement()
{
	// Opening paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	ZephyrToken stateName = ConsumeNextToken();
	if ( !DoesTokenMatchType( stateName, eTokenType::IDENTIFIER ) )
	{
		ReportError( "ChangeState must specify a target state" );
		return false;
	}

	// TODO: Check that the state name is valid


	// Closing paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) )
	{
		return false;
	}

	WriteConstantToCurChunk( ZephyrValue( stateName.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::CHANGE_STATE );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseIfStatement()
{
	// Write a placeholder for how many bytes the if block is so we can update it with the length later
	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT );
	int ifInstructionCountIdx = m_curBytecodeChunk->AddConstant( ZephyrValue( 0.f ) );
	m_curBytecodeChunk->WriteByte( ifInstructionCountIdx );

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) ) return false;

	if ( !ParseExpression( GetNextValueTypeInExpression() ) ) return false;

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) ) return false;

	WriteOpCodeToCurChunk( eOpCode::IF );

	int preIfBlockByteCount = (int)m_curBytecodeChunk->GetCode().size();

	if ( !ParseBlock() ) return false;

	// Set the number of bytes to jump to be the size of the if block plus 3 bytes
	// 2 for the constant declaration and 1 for the JUMP op to jump over the else statement
	m_curBytecodeChunk->SetConstantAtIdx( ifInstructionCountIdx, ZephyrValue( (float)( m_curBytecodeChunk->GetCode().size() - preIfBlockByteCount ) + 3.f ) );

	// Write a placeholder for the jump over the else block
	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT );
	int elseInstructionCountIdx = m_curBytecodeChunk->AddConstant( ZephyrValue( 0.f ) );
	m_curBytecodeChunk->WriteByte( elseInstructionCountIdx );

	WriteOpCodeToCurChunk( eOpCode::JUMP );

	// Check for else statement
	if ( GetCurToken().GetType() == eTokenType::ELSE )
	{
		AdvanceToNextToken();

		int preElseBlockByteCount = (int)m_curBytecodeChunk->GetCode().size();

		if ( !ParseBlock() ) return false;

		m_curBytecodeChunk->SetConstantAtIdx( elseInstructionCountIdx, ZephyrValue( (float)( m_curBytecodeChunk->GetCode().size() - preElseBlockByteCount ) ) );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::GenerateIfStatementBytecode( std::vector<ZephyrValue>& byteJumpCounts )
{
	// Write a placeholder for how many bytes the if block is so we can update it with the length later
	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT );
	int ifInstructionCountIdx = m_curBytecodeChunk->AddConstant( ZephyrValue( 0.f ) );
	m_curBytecodeChunk->WriteByte( ifInstructionCountIdx );

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) ) return false;

	if ( !ParseExpression( GetNextValueTypeInExpression() ) ) return false;

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) ) return false;

	WriteOpCodeToCurChunk( eOpCode::IF );

	int preIfBlockByteCount = (int)m_curBytecodeChunk->GetCode().size();

	if ( !ParseBlock() ) return false;

	// Set the number of bytes to jump to be the size of the if block plus 3 bytes
	// 2 for the constant declaration and 1 for the JUMP op to jump over the else statement
	byteJumpCounts.push_back( ZephyrValue( (float)( m_curBytecodeChunk->GetCode().size() - preIfBlockByteCount ) + 3.f ) );
	//m_curBytecodeChunk->SetConstantAtIdx( ifInstructionCountIdx, ZephyrValue( (float)( m_curBytecodeChunk->GetCode().size() - preIfBlockByteCount ) + 3.f ) );

	// Write a placeholder for the jump over the else block
	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT );
	int elseInstructionCountIdx = m_curBytecodeChunk->AddConstant( ZephyrValue( 0.f ) );
	m_curBytecodeChunk->WriteByte( elseInstructionCountIdx );

	WriteOpCodeToCurChunk( eOpCode::JUMP );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseAssignment()
{
	// Start at the identifier for TryToGetVariable to work properly
	BackupToLastToken();
	ZephyrToken identifier = GetCurToken();
	ZephyrToken nextToken = PeekNextToken();

	ZephyrValue value;
	switch ( nextToken.GetType() )
	{
		case eTokenType::EQUAL:
		{
			// Advance to the actual value for expression
			AdvanceToNextToken();
			AdvanceToNextToken();
			if ( !TryToGetVariable( identifier.GetData(), value ) )
			{
				ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
				return false;
			}

			if ( !ParseExpression( value.GetType() ) )
			{
				return false;
			}

			WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
			WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT );
		}
		break;

		// Special case to handle member assignment for Vec2
		case eTokenType::PERIOD:
		{
			if ( !TryToGetVariable( identifier.GetData(), value ) )
			{
				ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
				return false;
			}

			AdvanceToNextToken();
			AdvanceToNextToken();
			ZephyrToken member = ConsumeNextToken();

			if ( !ConsumeExpectedNextToken( eTokenType::EQUAL ) )
			{
				ReportError( Stringf( "Assignment to variable '%s.%s' expected a '=' sign after the variable name", identifier.GetData().c_str(), member.GetData().c_str() ) );
				return false;
			}

			if ( !ParseExpression( eValueType::NUMBER ) )
			{
				return false;
			}

			WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
			WriteConstantToCurChunk( ZephyrValue( member.GetData() ) );
			WriteOpCodeToCurChunk( eOpCode::MEMBER_ASSIGNMENT );
		}
		break;

		default:
		{
			ReportError( Stringf( "Assignment to variable '%s' expected a '=' sign after the variable name", identifier.GetData().c_str() ) );
			return false;
		}
	}

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

		case eValueType::VEC2:
		{
			switch ( curToken.GetType() )
			{
				case eTokenType::MINUS:			return WriteOpCodeToCurChunk( eOpCode::NEGATE );
			}
		}
		break;
		
		case eValueType::BOOL:
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
				case eTokenType::AND:			return WriteOpCodeToCurChunk( eOpCode::AND );
				case eTokenType::OR:			return WriteOpCodeToCurChunk( eOpCode::OR );
				default: ReportError( Stringf( "Invalid operation '%s' for Number variables", ToString( curToken.GetType() ).c_str() ) ); return false;
			}
		}
		break;

		case eValueType::VEC2:
		{
			switch ( curToken.GetType() )
			{
				case eTokenType::PLUS:			return WriteOpCodeToCurChunk( eOpCode::ADD );
				case eTokenType::MINUS:			return WriteOpCodeToCurChunk( eOpCode::SUBTRACT );
				case eTokenType::STAR:			return WriteOpCodeToCurChunk( eOpCode::MULTIPLY );
				case eTokenType::BANG_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::NOT_EQUAL );
				case eTokenType::EQUAL_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::EQUAL );
				case eTokenType::AND:			return WriteOpCodeToCurChunk( eOpCode::AND );
				case eTokenType::OR:			return WriteOpCodeToCurChunk( eOpCode::OR );
				default: ReportError( Stringf( "Invalid operation '%s' for Vec2 variables", ToString( curToken.GetType() ).c_str() ) ); return false;
			}
		}
		break;

		case eValueType::BOOL:
		{
			switch ( curToken.GetType() )
			{
				case eTokenType::BANG_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::NOT_EQUAL );
				case eTokenType::EQUAL_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::EQUAL );
				case eTokenType::AND:			return WriteOpCodeToCurChunk( eOpCode::AND );
				case eTokenType::OR:			return WriteOpCodeToCurChunk( eOpCode::OR );
				default: ReportError( Stringf( "Invalid operation '%s' for Bool variables", ToString( curToken.GetType() ).c_str() ) ); return false;
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
				case eTokenType::AND:			return WriteOpCodeToCurChunk( eOpCode::AND );
				case eTokenType::OR:			return WriteOpCodeToCurChunk( eOpCode::OR );
				default: ReportError( Stringf( "Invalid operation '%s' for String variables", ToString( curToken.GetType() ).c_str() ) ); return false;
			}
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseNumberConstant()
{
	ZephyrToken curToken = ConsumeNextToken();

	return WriteConstantToCurChunk( (NUMBER_TYPE)atof( curToken.GetData().c_str() ) );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseVec2Constant()
{
	if ( !ConsumeExpectedNextToken( eTokenType::VEC2 ) ) { return false; }

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) ) { return false; }

	// Parse x value
	if ( !ParseExpression( eValueType::NUMBER ) )
	{
		return false;
	}

	if ( !ConsumeExpectedNextToken( eTokenType::COMMA ) ) { return false; }

	// Parse y value
	if ( !ParseExpression( eValueType::NUMBER ) )
	{
		return false;
	}

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) ) { return false; }

	return WriteOpCodeToCurChunk( eOpCode::CONSTANT_VEC2 );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBoolConstant( bool value )
{
	ZephyrToken curToken = ConsumeNextToken();

	return WriteConstantToCurChunk( value );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseStringConstant()
{
	ZephyrToken curToken = ConsumeNextToken();

	return WriteConstantToCurChunk( ZephyrValue( curToken.GetData() ) );
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
		// Check if this could be a vec2 member accessor
		if ( expectedType == eValueType::NUMBER
			 && value.GetType() == eValueType::VEC2 )
		{
			if( ConsumeExpectedNextToken( eTokenType::PERIOD ) )
			{
				ZephyrToken memberName = ConsumeNextToken();
				if ( memberName.GetData() == "x" 
					 || memberName.GetData() == "y" )
				{
					WriteConstantToCurChunk( ZephyrValue( curToken.GetData() ) );
					WriteConstantToCurChunk( ZephyrValue( memberName.GetData() ) );
					WriteOpCodeToCurChunk( eOpCode::GET_MEMBER_VARIABLE_VALUE );

					return true;
				}
				else
				{
					ReportError( Stringf( "'%s' is not a member of Vec2", curToken.GetData().c_str() ) );
					return false;
				}
			}
		}


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
		case eTokenType::MINUS:				return ParseUnaryExpression( expectedType );
		case eTokenType::BANG:				return ParseUnaryExpression( expectedType );

		// TODO: do type checking here without breaking mixed type and/or statements
		case eTokenType::CONSTANT_NUMBER:	
		{
			//if ( expectedType == eValueType::NUMBER )
			//{
				return ParseNumberConstant();
			//}

			//ReportError( Stringf( "Expected variable of type '%s' but found type 'Number'", ToString(expectedType).c_str() ) );
			//return false;
		}
		break;

		case eTokenType::VEC2:		
		{
			//if ( expectedType == eValueType::VEC2 )
			//{
				return ParseVec2Constant();
			//}

			//ReportError( Stringf( "Expected variable of type '%s' but found type 'Vec2'", ToString( expectedType ).c_str() ) );
			//return false;
		}
		break;

		case eTokenType::TRUE:	return ParseBoolConstant( true );
		case eTokenType::FALSE:	return ParseBoolConstant( false );

		case eTokenType::CONSTANT_STRING:	
		{
			//if ( expectedType == eValueType::STRING )
			//{
				return ParseStringConstant();
			//}

			//ReportError( Stringf( "Expected variable of type '%s' but found type 'String'", ToString( expectedType ).c_str() ) );
			//return false;
		}

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
		case eTokenType::AND:
		case eTokenType::OR:
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
		case eTokenType::AND:				return eOpPrecedenceLevel::AND;
		case eTokenType::OR:				return eOpPrecedenceLevel::OR;
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
			case eTokenType::CONSTANT_NUMBER:	return eValueType::NUMBER;
			case eTokenType::VEC2:				return eValueType::VEC2;
			case eTokenType::TRUE:				return eValueType::BOOL;
			case eTokenType::FALSE:				return eValueType::BOOL;
			case eTokenType::CONSTANT_STRING:	return eValueType::STRING;
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
ZephyrToken ZephyrParser::PeekNextToken() const
{
	if ( IsAtEnd() )
	{
		return m_tokens[m_tokens.size() - 1];
	}

	return m_tokens[m_curTokenIdx + 1];
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::PeekNextNextToken() const
{
	if ( IsAtEnd() 
		 || ( m_curTokenIdx + 2 ) > (int)m_tokens.size() - 1 )
	{
		return m_tokens[m_tokens.size() - 1];
	}

	return m_tokens[m_curTokenIdx + 2];
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::ConsumeNextToken()
{
	if ( IsAtEnd() )
	{
		return m_tokens[m_tokens.size() - 1] ;
	}

	int tokenIdx = m_curTokenIdx;
	++m_curTokenIdx;

	if ( !IsAtEnd()
		 && m_tokens[m_curTokenIdx].GetType() == eTokenType::ERROR_TOKEN )
	{
		ReportError( m_tokens[m_curTokenIdx].GetData() );
	}

	return m_tokens[tokenIdx];
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::AdvanceToNextToken()
{
	if ( IsAtEnd() )
	{
		 return;
	}

	++m_curTokenIdx;

	if ( !IsAtEnd()
		 && m_tokens[m_curTokenIdx].GetType() == eTokenType::ERROR_TOKEN )
	{
		ReportError( m_tokens[m_curTokenIdx].GetData() );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::BackupToLastToken()
{
	--m_curTokenIdx;
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
ZephyrToken ZephyrParser::GetLastToken() const
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
		case eValueType::VEC2:	 value = ZephyrValue( Vec2::ZERO ); break;
		case eValueType::BOOL:	 value = ZephyrValue( false ); break;
		case eValueType::STRING: value = ZephyrValue( std::string("") ); break;
	}

	m_curBytecodeChunk->SetVariable( identifier.GetData(), value );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::TryToGetVariable( const std::string& identifier, ZephyrValue& out_value )
{
	bool foundValue = m_curBytecodeChunk->TryToGetVariable( identifier, out_value );

	if ( !foundValue
		 && m_curBytecodeChunk != m_stateMachineBytecodeChunk )
	{
		foundValue = m_stateMachineBytecodeChunk->TryToGetVariable( identifier, out_value );
	}

	// Check for member accessor
	if ( foundValue && out_value.GetType() == eValueType::VEC2 )
	{
		ZephyrToken nextToken = PeekNextToken();
		if ( nextToken.GetType() == eTokenType::PERIOD )
		{
			ZephyrToken member = PeekNextNextToken();

			if ( member.GetData() == "x" ) 
			{
				out_value = ZephyrValue( out_value.GetAsVec2().x );
			}
			else if ( member.GetData() == "y" ) 
			{ 
				out_value = ZephyrValue( out_value.GetAsVec2().x );
			}
			else
			{
				ReportError( Stringf( "%s is not a member of Vec2", member.GetData().c_str() ) );
				return false;
			}
		}

	}

	return foundValue;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::IsAtEnd() const
{
	return m_curTokenIdx >= (int)m_tokens.size();
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::GetCurToken() const
{
	return m_tokens[m_curTokenIdx];
}


//-----------------------------------------------------------------------------------------------
eTokenType ZephyrParser::GetCurTokenType() const
{
	return m_tokens[m_curTokenIdx].GetType();
}


//-----------------------------------------------------------------------------------------------
int ZephyrParser::GetCurTokenLineNum() const
{
	return m_tokens[m_curTokenIdx].GetLineNum();
}

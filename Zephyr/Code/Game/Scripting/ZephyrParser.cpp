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
	CreateStateMachineBytecodeChunk();

	ZephyrToken nextToken = GetCurToken();
	while ( !DoesTokenMatchType( nextToken, eTokenType::END_OF_FILE ) )
	{
		if ( !ParseStatement() )
		{
			return new ZephyrScriptDefinition( nullptr, m_bytecodeChunks );
		}

		nextToken = GetCurToken();
	}
	
	// Check for any chunks with too many constants
	bool anyErrorChunks = false;
	for ( auto const& bytecodeChunk : m_bytecodeChunks )
	{
		if ( bytecodeChunk.second->GetNumConstants() > 254 )
		{
			std::string bytecodeChunkType = ToString( bytecodeChunk.second->GetType() );
			ReportError( Stringf( "%s %s contains too many constants. Try to break up into smaller functions", bytecodeChunkType.c_str(), bytecodeChunk.first.c_str(), bytecodeChunkType.c_str() ) );
			anyErrorChunks = true;
		}

		for ( auto const& eventChunk : bytecodeChunk.second->GetEventBytecodeChunks() )
		{
			if ( eventChunk.second->GetNumConstants() > 254 )
			{
				std::string eventChunkType = ToString( eventChunk.second->GetType() );
				ReportError( Stringf( "%s %s contains too many constants. Try to break up into smaller functions", eventChunkType.c_str(), eventChunk.first.c_str(), eventChunkType.c_str() ) );
				anyErrorChunks = true;
			}
		}
	}

	if ( anyErrorChunks )
	{
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
	
	// Save reference to this entity into global state
	m_stateMachineBytecodeChunk->SetVariable( PARENT_ENTITY_NAME, ZephyrValue( (EntityId)-1 ) );

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
	ZephyrToken curToken = ConsumeCurToken();

	// Error if this statement is invalid for chunk type
	if ( !IsStatementValidForChunk( curToken.GetType(), m_curBytecodeChunk->GetType() ) )
	{
		return false;
	}

	switch ( curToken.GetType() )
	{
		case eTokenType::STATE:				
		{
			return ParseStateDefinition();
		}
		break;

		case eTokenType::FUNCTION:
		{
			return ParseFunctionDefinition();
		}
		
		
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

		case eTokenType::ENTITY:
		{
			if ( !ParseVariableDeclaration( eValueType::ENTITY ) )
			{
				return false;
			}
		}
		break;
		
		case eTokenType::ON_ENTER:
		case eTokenType::ON_EXIT:
		case eTokenType::ON_UPDATE:
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
		
		case eTokenType::IDENTIFIER:
		{
			// Check if this is a function name by looking for opening paren
			if ( GetCurTokenType() == eTokenType::PARENTHESIS_LEFT  )
			{
				if ( !ParseFunctionCall() )
				{
					return false;
				}
				
			}
			else if ( GetCurTokenType() == eTokenType::PERIOD )
			{
				BackupToLastToken();
				return ParseMemberAccessor();
			}
			else
			{
				if ( !ParseAssignment() )
				{
					return false;
				}
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

	AdvanceThroughAllMatchingTokens( eTokenType::SEMICOLON );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseStateDefinition()
{
	ZephyrToken curToken = ConsumeCurToken();
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


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseFunctionDefinition()
{
	ZephyrToken functionNameToken = ConsumeCurToken();
	if ( functionNameToken.GetType() != eTokenType::IDENTIFIER )
	{
		ReportError( "Function must be specified in the form: Function Example()" );
		return false;
	}

	// Create this function chunk so params are saved inside
	bool succeeded = CreateBytecodeChunk( functionNameToken.GetData(), eBytecodeChunkType::EVENT );
	if ( !succeeded )
	{
		return false;
	}

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	ZephyrToken curToken = ConsumeCurToken();
	while ( curToken.GetType() != eTokenType::PARENTHESIS_RIGHT )
	{
		switch ( curToken.GetType() )
		{
			case eTokenType::NUMBER: if ( !ParseVariableDeclaration( eValueType::NUMBER ) ) { return false; } break;
			case eTokenType::VEC2:	 if ( !ParseVariableDeclaration( eValueType::VEC2   ) ) { return false; } break;
			case eTokenType::BOOL:   if ( !ParseVariableDeclaration( eValueType::BOOL   ) ) { return false; } break;
			case eTokenType::STRING: if ( !ParseVariableDeclaration( eValueType::STRING ) ) { return false; } break;
			case eTokenType::ENTITY: if ( !ParseVariableDeclaration( eValueType::ENTITY ) ) { return false; } break;

			default:
			{
				ReportError( Stringf( "Only variables can be declared inside Function() definition, found %s", ToString( curToken.GetType() ).c_str() ) );
				return false;
			}
		}

		AdvanceToNextTokenIfTypeMatches( eTokenType::COMMA );
		curToken = ConsumeCurToken();
	}

	/*if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) )
	{
		return false;
	}*/

	succeeded = ParseBlock();

	FinalizeCurBytecodeChunk();

	return succeeded;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseVariableDeclaration( const eValueType& varType )
{
	ZephyrToken identifier = ConsumeCurToken();
	if ( !DoesTokenMatchType( identifier, eTokenType::IDENTIFIER ) )
	{
		ReportError( Stringf( "Expected variable name after '%s'", ToString( varType ).c_str() ) );
		return false;
	}

	if ( !DeclareVariable( identifier, varType ) )
	{
		return false;
	}

	ZephyrToken curToken = GetCurToken();
	switch ( curToken.GetType() )
	{
		case eTokenType::EQUAL:
		{
			AdvanceToNextToken();

			if ( !ParseExpression() )
			{
				return false;
			}

			WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
			WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT );
		}
		break;

		default:
		{
			switch ( varType )
			{
				case eValueType::NUMBER: WriteConstantToCurChunk( ZephyrValue( 0.f ) ); break;
				case eValueType::BOOL:	 WriteConstantToCurChunk( ZephyrValue( false ) ); break;
				case eValueType::STRING: WriteConstantToCurChunk( ZephyrValue( "" ) ); break;
				case eValueType::ENTITY: WriteConstantToCurChunk( ZephyrValue( (EntityId)-1 ) ); break;
				case eValueType::VEC2:	 WriteConstantToCurChunk( ZephyrValue( Vec2::ZERO ) ); break;
			}

			/*std::string errorMsg( "Unexpected '" );
			errorMsg += curToken.GetDebugName();
			errorMsg += "' seen, expected ';' or '=' after '";
			errorMsg +=	ToString( varType );
			errorMsg +=	" declaration";

			ReportError( errorMsg );
			return false;*/
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseFunctionCall()
{
	ZephyrToken functionName = GetLastToken();
	
	// Function call opening paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	//if ( !DoesTokenMatchType( eventName, eTokenType::IDENTIFIER ) )
	//{
	//	if ( DoesTokenMatchType( eventName, eTokenType::ON_ENTER ) )
	//	{
	//		ReportError( "OnEnter cannot be called from FireEvent, it's automatically called when entering a state" );
	//	}
	//	else if ( DoesTokenMatchType( eventName, eTokenType::ON_EXIT ) )
	//	{
	//		ReportError( "OnExit cannot be called from FireEvent, it's automatically called when exiting a state" );
	//	}
	//	else if ( DoesTokenMatchType( eventName, eTokenType::ON_UPDATE ) )
	//	{
	//		ReportError( "OnUpdate cannot be called from FireEvent, it's automatically called when the entity updates" );
	//	}
	//	else
	//	{
	//		ReportError( "FireEvent must specify an event to call in parentheses" );
	//	}

	if ( !ParseEventArgs() )
	{
		return false;
	}
	
	// We should be one token past the closing paren
	if ( GetLastToken().GetType() != eTokenType::PARENTHESIS_RIGHT )
	{
		ReportError( "Expected ')' after parameter list for function call" );
		return false;
	}

	WriteConstantToCurChunk( ZephyrValue( functionName.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::FUNCTION_CALL );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseEventArgs()
{
	ZephyrToken identifier = ConsumeCurToken();
	int paramCount = 0;

	std::vector<std::string> identifierNames;
	std::vector<std::string> identifierParamNames;

	while ( identifier.GetType() == eTokenType::IDENTIFIER )
	{
		if ( !ConsumeExpectedNextToken( eTokenType::COLON ) )
		{
			ReportError( "Parameter to event must be in the form, var: value" );
			return false;
		}

		ZephyrToken valueToken = GetCurToken();
		switch ( valueToken.GetType() )
		{
			case eTokenType::CONSTANT_NUMBER:
			case eTokenType::VEC2:
			case eTokenType::ENTITY:
			case eTokenType::TRUE:
			case eTokenType::FALSE:
			case eTokenType::NULL_TOKEN:
			case eTokenType::CONSTANT_STRING:
			{
				if ( !ParseExpression() )
				{
					return false;
				}
			}
			break;
			
			case eTokenType::IDENTIFIER:
			{
				// Only pass single identifiers by reference
				eTokenType nextType = PeekNextToken().GetType();
				if ( nextType == eTokenType::COMMA
					 || nextType == eTokenType::PARENTHESIS_RIGHT 
					 || nextType == eTokenType::IDENTIFIER )
				{
					identifierNames.push_back( valueToken.GetData() );
					identifierParamNames.push_back( identifier.GetData() );
				}

				if ( !ParseExpression() )
				{
					return false;
				}
			}
			break;

			default:
			{
				ReportError( "Must set parameter equal to a value in the form, var: value" );
				return false;
			}
		}

		WriteConstantToCurChunk( identifier.GetData() );
		++paramCount;

		AdvanceToNextTokenIfTypeMatches( eTokenType::COMMA );

		identifier = ConsumeCurToken();
	}

	WriteConstantToCurChunk( ZephyrValue( (float)paramCount ) );

	for ( int identifierIdx = 0; identifierIdx < (int)identifierParamNames.size(); ++identifierIdx )
	{
		WriteConstantToCurChunk( identifierNames[identifierIdx] );
		WriteConstantToCurChunk( identifierParamNames[identifierIdx] );
	}

	WriteConstantToCurChunk( ZephyrValue( (float)identifierParamNames.size() ) );

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

	ZephyrToken stateName = ConsumeCurToken();
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

	if ( !ParseExpression() ) return false;

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
bool ZephyrParser::ParseAssignment()
{
	//// Advance past = sign
	//AdvanceToNextToken();

	//if ( !ParseExpression() )
	//{
	//	return false;
	//}

	//WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
	//WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT );

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
			/*if ( !TryToGetVariable( identifier.GetData(), value ) )
			{
				ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
				return false;
			}*/

			if ( !ParseExpression() )
			{
				return false;
			}

			WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
			WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT );
		}
		break;

		//// Special case to handle member assignment for Vec2
		//case eTokenType::PERIOD:
		//{
		//	eValueType valType;
		//	if ( !TryToGetVariableType( identifier.GetData(), valType ) )
		//	{
		//		ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
		//		return false;
		//	}

		//	// Make sure this variable can have members
		//	if ( valType != eValueType::VEC2 )
		//	{
		//		ReportError( Stringf( "Variable '%s' of type '%s' doesn't have any members to access", identifier.GetData().c_str(), ToString( valType ).c_str() ) );
		//		return false;
		//	}

		//	if ( !TryToGetVariable( identifier.GetData(), value ) )
		//	{
		//		ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
		//		return false;
		//	}	

		//	AdvanceToNextToken();
		//	AdvanceToNextToken();
		//	ZephyrToken member = ConsumeCurToken();

		//	if ( !ConsumeExpectedNextToken( eTokenType::EQUAL ) )
		//	{
		//		ReportError( Stringf( "Assignment to variable '%s.%s' expected a '=' sign after the variable name", identifier.GetData().c_str(), member.GetData().c_str() ) );
		//		return false;
		//	}

		//	if ( !ParseExpression() )
		//	{
		//		return false;
		//	}

		//	WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
		//	WriteConstantToCurChunk( ZephyrValue( member.GetData() ) );
		//	WriteOpCodeToCurChunk( eOpCode::MEMBER_ASSIGNMENT );
		//}
		//break;

		default:
		{
			ReportError( Stringf( "Assignment to variable '%s' expected a '=' sign after the variable name", identifier.GetData().c_str() ) );
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseMemberAssignment()
{
	BackupToLastToken();
	ZephyrToken identifier = GetCurToken();
	ZephyrToken nextToken = PeekNextToken();

	eValueType valType;
	if ( !TryToGetVariableType( identifier.GetData(), valType ) )
	{
		ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
		return false;
	}

	// Make sure this variable can have members
	/*if ( valType != eValueType::VEC2 )
	{
		ReportError( Stringf( "Variable '%s' of type '%s' doesn't have any members to access", identifier.GetData().c_str(), ToString( valType ).c_str() ) );
		return false;
	}

	ZephyrValue value;
	if ( !TryToGetVariable( identifier.GetData(), value ) )
	{
		ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
		return false;
	}*/

	AdvanceToNextToken();
	AdvanceToNextToken();
	ZephyrToken member = ConsumeCurToken();

	if ( !ConsumeExpectedNextToken( eTokenType::EQUAL ) )
	{
		ReportError( Stringf( "Assignment to variable '%s.%s' expected a '=' sign after the variable name", identifier.GetData().c_str(), member.GetData().c_str() ) );
		return false;
	}

	if ( !ParseExpression() )
	{
		return false;
	}

	WriteConstantToCurChunk( ZephyrValue( identifier.GetData() ) );
	WriteConstantToCurChunk( ZephyrValue( member.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::MEMBER_ASSIGNMENT );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseMemberAccessor()
{
	ZephyrToken topLevelObj = GetCurToken();
	
	// Advance to first period
	AdvanceToNextToken();
	
	int memberCount = 0;
	while ( GetCurTokenType() == eTokenType::PERIOD )
	{
		++memberCount;

		// Advance past period
		AdvanceToNextToken();

		ZephyrToken member = ConsumeCurToken();

		if ( member.GetType() != eTokenType::IDENTIFIER )
		{
			ReportError( Stringf( "Invalid symbol seen after '.': '%s' Only variable or function names can follow '.'", member.GetData().c_str() ) );
			return false;
		}

		WriteConstantToCurChunk( ZephyrValue( member.GetData() ) );
	}

	// Write number of accessors as number
	WriteConstantToCurChunk( ZephyrValue( topLevelObj.GetData() ) );
	WriteConstantToCurChunk( ZephyrValue( (float)memberCount ) );
		
	switch ( GetCurTokenType() )
	{
		case eTokenType::EQUAL:
		{
			AdvanceToNextToken();
			if ( !ParseExpression() )
			{
				return false;
			}

			WriteOpCodeToCurChunk( eOpCode::MEMBER_ASSIGNMENT );
		}
		break;

		case eTokenType::PARENTHESIS_LEFT:
		{
			AdvanceToNextToken();
			if ( !ParseEventArgs() )
			{
				return false;
			}

			// We should be one token past the closing paren
			if ( GetLastToken().GetType() != eTokenType::PARENTHESIS_RIGHT )
			{
				ReportError( "Expected ')' after parameter list for function call" );
				return false;
			}

			WriteOpCodeToCurChunk( eOpCode::MEMBER_FUNCTION_CALL );
		}
		break;

		// If something else is at current token, must just be an accessor in an expression
		default:
		{
			WriteOpCodeToCurChunk( eOpCode::MEMBER_ACCESSOR );
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
		return false;
	}

	curToken = GetCurToken();
	while ( precLevel <= GetPrecedenceLevel( curToken ) )
	{
		if ( !CallInfixFunction( curToken ) )
		{
			return false;
		}

		curToken = GetCurToken();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CallPrefixFunction( const ZephyrToken& token )
{
	switch ( token.GetType() )
	{
		case eTokenType::PARENTHESIS_LEFT:	return ParseParenthesesGroup();
		case eTokenType::MINUS:				return ParseUnaryExpression();
		case eTokenType::BANG:				return ParseUnaryExpression();

		case eTokenType::CONSTANT_NUMBER:	return ParseNumberConstant();
		case eTokenType::VEC2:				return ParseVec2Constant();
		case eTokenType::TRUE:				return ParseBoolConstant( true );
		case eTokenType::FALSE:				return ParseBoolConstant( false );
		case eTokenType::NULL_TOKEN:		return ParseEntityConstant();
		case eTokenType::CONSTANT_STRING:	return ParseStringConstant();
		case eTokenType::ENTITY:

		case eTokenType::IDENTIFIER:
		{
			if ( PeekNextToken().GetType() == eTokenType::EQUAL )
			{
				// TODO: advance just to back up?
				AdvanceToNextToken();
				return ParseAssignment();
			}

			// Need to keep checking for = sign here
			if ( PeekNextToken().GetType() == eTokenType::PERIOD )
			{
				return ParseMemberAccessor();
			}

			// Check if this is a function call
			/*if ( PeekNextToken().GetType() == eTokenType::PARENTHESIS_LEFT )
			{
				AdvanceToNextToken();
				return ParseFunctionCall();
			}*/

			return ParseIdentifierExpression();
		}
		break;
	}

	// TODO: Make this more descriptive
	ReportError( "Missing expression" );
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
		case eTokenType::BANG_EQUAL:
		case eTokenType::EQUAL_EQUAL:
		case eTokenType::GREATER:
		case eTokenType::GREATER_EQUAL:
		case eTokenType::LESS:
		case eTokenType::LESS_EQUAL:
		case eTokenType::AND:
		case eTokenType::OR:
		{
			return ParseBinaryExpression();
		}
		break;
		
		/*case eTokenType::EQUAL:
		{
			return ParseAssignment();
		}*/

		/*case eTokenType::PERIOD:
		{
			return ParseMemberAccessor();
		}*/

		/*case eTokenType::PARENTHESIS_LEFT: 
		{
			return ParseFunctionCall();
		}*/
	}

	// TODO: Make this more descriptive
	ReportError( "Missing expression" );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseParenthesesGroup()
{
	ConsumeCurToken();

	if ( !ParseExpression() )
	{
		return false;
	}
	
	return ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseUnaryExpression()
{
	ZephyrToken curToken = ConsumeCurToken();

	if ( !ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel::UNARY ) )
	{
		return false;
	}

	switch ( curToken.GetType() )
	{
		case eTokenType::MINUS:			return WriteOpCodeToCurChunk( eOpCode::NEGATE );
		case eTokenType::BANG:			return WriteOpCodeToCurChunk( eOpCode::NOT );
	}
		
	ReportError( Stringf( "Invalid unary operation '%s'", ToString( curToken.GetType() ).c_str() ) );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBinaryExpression()
{
	ZephyrToken curToken = ConsumeCurToken();

	eOpPrecedenceLevel precLevel = GetNextHighestPrecedenceLevel( curToken );

	if ( !ParseExpressionWithPrecedenceLevel( precLevel ) )
	{
		return false;
	}

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
	}

	ReportError( Stringf( "Invalid binary operation '%s'", ToString( curToken.GetType() ).c_str() ) );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseNumberConstant()
{
	ZephyrToken curToken = ConsumeCurToken();

	return WriteConstantToCurChunk( (NUMBER_TYPE)atof( curToken.GetData().c_str() ) );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseVec2Constant()
{
	if ( !ConsumeExpectedNextToken( eTokenType::VEC2 ) ) { return false; }

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) ) { return false; }

	// Parse x value
	if ( !ParseExpression() )
	{
		return false;
	}

	if ( !ConsumeExpectedNextToken( eTokenType::COMMA ) ) { return false; }

	// Parse y value
	if ( !ParseExpression() )
	{
		return false;
	}

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) ) { return false; }

	return WriteOpCodeToCurChunk( eOpCode::CONSTANT_VEC2 );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBoolConstant( bool value )
{
	ConsumeCurToken();

	return WriteConstantToCurChunk( value );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseEntityConstant()
{
	ConsumeCurToken();

	return WriteConstantToCurChunk( ZephyrValue( (EntityId)-1 ) );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseStringConstant()
{
	ZephyrToken curToken = ConsumeCurToken();

	return WriteConstantToCurChunk( ZephyrValue( curToken.GetData() ) );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseIdentifierExpression()
{
	ZephyrToken curToken = ConsumeCurToken();

	if ( curToken.GetType() != eTokenType::IDENTIFIER )
	{
		ReportError( Stringf( "Expected Identifier, but found '%s'", ToString( curToken.GetType() ).c_str() ) );
		return false;
	}

	/*ZephyrValue value;
	if ( !TryToGetVariable( curToken.GetData(), value ) )
	{
		ReportError( Stringf( "Undefined variable seen, '%s'", curToken.GetData().c_str() ) );
		return false;
	}*/
	
	WriteConstantToCurChunk( ZephyrValue( curToken.GetData() ) );
	WriteOpCodeToCurChunk( eOpCode::GET_VARIABLE_VALUE );

	return true;
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
		case eTokenType::PERIOD:			return eOpPrecedenceLevel::CALL;
		case eTokenType::PARENTHESIS_LEFT:	return eOpPrecedenceLevel::CALL;
		case eTokenType::EQUAL:				return eOpPrecedenceLevel::ASSIGNMENT;
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
ZephyrToken ZephyrParser::ConsumeCurToken()
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
void ZephyrParser::AdvanceThroughAllMatchingTokens( eTokenType expectedType )
{
	while ( GetCurToken().GetType() == expectedType )
	{
		AdvanceToNextToken();
	}
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ConsumeExpectedNextToken( eTokenType expectedType )
{
	ZephyrToken curToken = ConsumeCurToken();

	if ( !DoesTokenMatchType( curToken, expectedType ) )
	{
		ReportError( Stringf( "Expected '%s'", ToString( expectedType ).c_str() ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::IsStatementValidForChunk( eTokenType statementToken, eBytecodeChunkType chunkType )
{
	switch ( statementToken )
	{
		// Valid to define anywhere
		case eTokenType::STATE:
		case eTokenType::FUNCTION:
		case eTokenType::NUMBER:
		case eTokenType::VEC2:
		case eTokenType::BOOL:
		case eTokenType::STRING:
		case eTokenType::ENTITY:
		case eTokenType::BRACE_RIGHT:
		{
			return true;
		}
		break;
		
		// Valid to define in state only
		case eTokenType::ON_ENTER:
		case eTokenType::ON_EXIT:
		case eTokenType::ON_UPDATE:
		{
			if ( chunkType != eBytecodeChunkType::STATE )
			{
				ReportError( Stringf( "'%s' can only be defined inside a State", ToString( statementToken ).c_str() ) );
				return false;
			}

			return true;
		}
		break;

		// Valid to define in event only
		case eTokenType::CHANGE_STATE:
		case eTokenType::IF:
		case eTokenType::RETURN:
		case eTokenType::IDENTIFIER:
		{
			if ( chunkType != eBytecodeChunkType::EVENT )
			{
				ReportError( Stringf( "'%s' can only be defined inside an Event", ToString( statementToken ).c_str() ) );
				return false;
			}

			return true;
		}
		break;

		default:
		{
			ReportError( Stringf( "Unknown statement starting with '%s' seen", ToString( statementToken ).c_str() ) );
			return false;
		}
	}
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
bool ZephyrParser::DeclareVariable( const ZephyrToken& identifier, const eValueType& varType )
{
	return DeclareVariable( identifier.GetData(), varType );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::DeclareVariable( const std::string& identifier, const eValueType& varType )
{
	// m_curBytecodeChunk will be the global state machine if outside a state declaration, should never be null
	ZephyrValue value;
	switch ( varType )
	{
		case eValueType::NUMBER: value = ZephyrValue( 0.f ); break;
		case eValueType::VEC2:	 value = ZephyrValue( Vec2::ZERO ); break;
		case eValueType::BOOL:	 value = ZephyrValue( false ); break;
		case eValueType::STRING: value = ZephyrValue( std::string( "" ) ); break;
		case eValueType::ENTITY: 
		{
			// Cannot redefine parent entity
			if ( identifier == PARENT_ENTITY_NAME )
			{
				ReportError( Stringf( "Cannot define an entity named '%s'. That name is reserved to reference the parent entity for this script.", PARENT_ENTITY_NAME.c_str() ) );
				return false;
			}

			value = ZephyrValue( (EntityId)-1 ); break;
		}
	}

	m_curBytecodeChunk->SetVariable( identifier, value );
	return true;
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

	//// Check for member accessor for Vec2
	//if ( foundValue && out_value.GetType() == eValueType::VEC2 )
	//{
	//	ZephyrToken nextToken = PeekNextToken();
	//	if ( nextToken.GetType() == eTokenType::PERIOD )
	//	{
	//		ZephyrToken member = PeekNextNextToken();

	//		if ( member.GetData() == "x" ) 
	//		{
	//			out_value = ZephyrValue( out_value.GetAsVec2().x );
	//		}
	//		else if ( member.GetData() == "y" ) 
	//		{ 
	//			out_value = ZephyrValue( out_value.GetAsVec2().x );
	//		}
	//		else
	//		{
	//			ReportError( Stringf( "%s is not a member of Vec2", member.GetData().c_str() ) );
	//			return false;
	//		}
	//	}
	//}

	return foundValue;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::TryToGetVariableType( const std::string& identifier, eValueType& out_varType )
{
	ZephyrValue val;
	bool foundValue = m_curBytecodeChunk->TryToGetVariable( identifier, val );

	if ( !foundValue
		 && m_curBytecodeChunk != m_stateMachineBytecodeChunk )
	{
		foundValue = m_stateMachineBytecodeChunk->TryToGetVariable( identifier, val );
	}

	if ( foundValue )
	{
		out_varType = val.GetType();
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

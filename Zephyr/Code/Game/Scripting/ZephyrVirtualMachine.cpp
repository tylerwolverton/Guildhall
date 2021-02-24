#include "Game/Scripting/ZephyrVirtualMachine.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrVirtualMachine::ZephyrVirtualMachine()
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk,
												   ZephyrValueMap* globalVariables,
												   Entity* parentEntity,
												   EventArgs* eventArgs,
												   ZephyrValueMap* stateVariables )
{
	if ( !parentEntity->IsScriptValid() )
	{
		return;
	}

	m_parentEntity = parentEntity;
	m_globalVariables = globalVariables;
	m_stateVariables = stateVariables;
	CopyEventArgVariables( eventArgs );

	// Event variables don't need to be persisted after this call, so save a copy as local variables
	// TODO: Account for scopes inside if statements, etc.?
	std::map<std::string, ZephyrValue> localVariables;
	if ( bytecodeChunk.GetType() == eBytecodeChunkType::STATE 
		||  bytecodeChunk.GetType() == eBytecodeChunkType::EVENT )
	{
		localVariables = bytecodeChunk.GetVariables();
	}

	int byteIdx = 0;
	while ( byteIdx < bytecodeChunk.GetNumBytes() )
	{
		byte instruction = bytecodeChunk.GetByte( byteIdx++ );
		eOpCode opCode = ByteToOpCode( instruction );
		switch ( opCode )
		{
			case eOpCode::CONSTANT:
			{
				int constIdx = bytecodeChunk.GetByte( byteIdx++ );
				ZephyrValue constant = bytecodeChunk.GetConstant( constIdx );
				PushConstant( constant );
			}
			break;

			case eOpCode::DEFINE_VARIABLE:
			{
				ZephyrValue constant = PopConstant();
				localVariables[constant.GetAsString()] = PopConstant();
			}
			break;

			case eOpCode::GET_VARIABLE_VALUE:
			{
				ZephyrValue variableName = PopConstant();
				PushConstant( GetVariableValue( variableName.GetAsString(), localVariables ) );
			}
			break;

			case eOpCode::GET_MEMBER_VARIABLE_VALUE:
			{ 
				ZephyrValue memberName = PopConstant();
				ZephyrValue objectName = PopConstant();

				ZephyrValue objectVal = GetVariableValue( objectName.GetAsString(), localVariables );
				
				if ( objectVal.GetType() == eValueType::VEC2 )
				{
					if ( memberName.GetAsString() == "x" ) { PushConstant( objectVal.GetAsVec2().x ); }
					else if ( memberName.GetAsString() == "y" ) { PushConstant( objectVal.GetAsVec2().y ); }
				}
				else if ( objectVal.GetType() == eValueType::ENTITY )
				{
					//if( !DoesEntityHaveMember( memberName.GetAsString() ) )
					//{
						// Throw runtime error
					//}
					
					//PushConstant( GetConstantFromEntity( objectVal.GetAsEntity() ) );
				}
				else
				{
					// Throw runtime error
				}
			}
			break;

			case eOpCode::ASSIGNMENT:
			{
				ZephyrValue variableName = PopConstant(); 
				ZephyrValue constantValue = PeekConstant();
				AssignToVariable( variableName.GetAsString(), constantValue, localVariables );
			}
			break;

			case eOpCode::MEMBER_ASSIGNMENT:
			{
				ZephyrValue memberName = PopConstant();
				ZephyrValue variableName = PopConstant();
				ZephyrValue constantValue = PeekConstant();
				AssignToMemberVariable( variableName.GetAsString(), memberName.GetAsString(), constantValue, localVariables );
			}
			break;

			case eOpCode::MEMBER_ACCESSOR:
			{
				ZephyrValue memberCountZephyr = PopConstant();
				int memberCount = (int)memberCountZephyr.GetAsNumber();
				
				ZephyrValue baseObjName = PopConstant();

				// Pull all members from the constant stack into a temp buffer for processing 
				// so in case there is a member error the stack is left in a good(ish) state 
				// Note: the members will be in reverse order, so accoutn for that
				std::vector<std::string> memberNames;
				memberNames.resize( memberCount );
				for ( int memberIdx = memberCount - 1; memberIdx >= 0; --memberIdx )
				{
					memberNames[memberIdx] = PopConstant().GetAsString();
				}

				// Find base object in this bytecode chunk
				ZephyrValue memberVal = GetVariableValue( baseObjName.GetAsString(), localVariables );
				if ( IsErrorValue( memberVal ) )
				{
					return;
				}

				// Process accessors excluding the final component, since that needs to be handled separately
				for ( int memberNameIdx = 0; memberNameIdx < (int)memberNames.size()-1; ++memberNameIdx )
				{
					//std::string& memberName = memberNames[memberNameIdx];


	
				}

				// Push final member to top of constant stack
				const std::string& lastMemberName = memberNames[memberCount - 1];

				switch ( memberVal.GetType() )
				{
					case eValueType::BOOL:
					case eValueType::NUMBER:
					case eValueType::STRING:
					{
						ReportError( Stringf( "Variable of type %s can't have members. Tried to access '%s'",
											  ToString( memberVal.GetType() ).c_str(),
											  lastMemberName.c_str() ) );
						return;
					}
					break;

					case eValueType::VEC2:
					{
						if		( lastMemberName == "x" ) { PushConstant( memberVal.GetAsVec2().x ); }
						else if ( lastMemberName == "y" ) { PushConstant( memberVal.GetAsVec2().y ); }
						else
						{
							ReportError( Stringf( "'%s' is not a member of Vec2", lastMemberName.c_str() ) );
						}
					}
					break;

					case eValueType::ENTITY:
					{
						ZephyrValue val = GetGlobalVariableFromEntity( memberVal.GetAsEntity(), lastMemberName );
						if ( IsErrorValue( val ) )
						{
							std::string entityVarName = memberCount > 1 ? memberNames[memberCount - 2] : baseObjName.GetAsString();

							ReportError( Stringf( "Variable '%s' is not a member of Entity '%s'", lastMemberName.c_str(), entityVarName.c_str() ) );
							return;
						}

						PushConstant( val );
					}
					break;
				}
			}
			break;

			case eOpCode::CONSTANT_VEC2:
			{
				ZephyrValue yValue = PopConstant();
				ZephyrValue xValue = PopConstant();

				ZephyrValue value( Vec2( xValue.GetAsNumber(), yValue.GetAsNumber() ) );
				PushConstant( value );
			}
			break;

			case eOpCode::RETURN:
			{
				// Stop processing this bytecode chunk
				return;
			}
			break;

			case eOpCode::IF:
			{
				ZephyrValue expression = PopConstant();
				ZephyrValue ifBlockByteCount = PopConstant();

				// The if statement is false, jump over the bytes corresponding to that code block
				if ( !expression.IsTrue() )
				{
					byteIdx += (int)ifBlockByteCount.GetAsNumber();
				}
			}
			break;

			case eOpCode::JUMP:
			{
				ZephyrValue numBytesToJump = PopConstant();
				byteIdx += (int)numBytesToJump.GetAsNumber();
			}
			break;

			case eOpCode::AND:
			{
				ZephyrValue rightVal = PopConstant();
				ZephyrValue leftVal = PopConstant();

				if ( !leftVal.IsTrue() 
					 || !rightVal.IsTrue() )
				{
					PushConstant( ZephyrValue( false ) );
				}
				else
				{
					PushConstant( ZephyrValue( true ) );
				}
			}
			break;

			case eOpCode::OR:
			{
				ZephyrValue rightVal = PopConstant();
				ZephyrValue leftVal = PopConstant();

				if ( !leftVal.IsTrue()
					 && !rightVal.IsTrue() )
				{
					PushConstant( ZephyrValue( false ) );
				}
				else
				{
					PushConstant( ZephyrValue( true ) );
				}
			}
			break;

			case eOpCode::NEGATE:
			{
				ZephyrValue a = PopConstant();
				if ( a.GetType() == eValueType::NUMBER )
				{
					PushConstant( -a.GetAsNumber() );
				}
				else if ( a.GetType() == eValueType::VEC2 )
				{
					PushConstant( -a.GetAsVec2() );
				}
			}
			break;

			case eOpCode::NOT:
			{
				ZephyrValue a = PopConstant();
				if ( a.GetType() == eValueType::NUMBER )
				{
					if ( IsNearlyEqual( a.GetAsNumber(), 0.f, .00001f ) )
					{
						PushConstant( ZephyrValue( true ) );
					}
					else
					{
						PushConstant( ZephyrValue( false ) );
					}
				}
				else if ( a.GetType() == eValueType::BOOL )
				{
					PushConstant( !a.GetAsBool() );
				}
				else if ( a.GetType() == eValueType::STRING )
				{
					if ( a.GetAsString().empty() )
					{
						PushConstant( ZephyrValue( true ) );
					}
					else
					{
						PushConstant( ZephyrValue( false ) );
					}
				}
			}
			break;

			case eOpCode::ADD:
			case eOpCode::SUBTRACT:
			case eOpCode::MULTIPLY:
			case eOpCode::DIVIDE:
			case eOpCode::NOT_EQUAL:
			case eOpCode::EQUAL:
			case eOpCode::GREATER:
			case eOpCode::GREATER_EQUAL:
			case eOpCode::LESS:
			case eOpCode::LESS_EQUAL:
			{
				ZephyrValue b = PopConstant();
				ZephyrValue a = PopConstant();
				PushBinaryOp( a, b, opCode );
			}
			break;

			case eOpCode::FUNCTION_CALL:
			{
				ZephyrValue eventName = PopConstant();
				EventArgs args;
				args.SetValue( "entity", (void*)parentEntity );

				ZephyrValue paramCount = PopConstant();

				for ( int paramIdx = 0; paramIdx < (int)paramCount.GetAsNumber(); ++paramIdx )
				{
					ZephyrValue param = PopConstant();
					ZephyrValue value = PopConstant();

					switch ( value.GetType() )
					{
						case eValueType::BOOL:		args.SetValue( param.GetAsString(), value.GetAsBool() ); break;
						case eValueType::NUMBER:	args.SetValue( param.GetAsString(), value.GetAsNumber() ); break;
						case eValueType::VEC2:		args.SetValue( param.GetAsString(), value.GetAsVec2() ); break;
						case eValueType::STRING:	args.SetValue( param.GetAsString(), value.GetAsString() ); break;
						case eValueType::ENTITY:	args.SetValue( param.GetAsString(), value.GetAsEntity() ); break;
						default: ERROR_AND_DIE( Stringf( "Unimplemented event arg type '%s'", ToString( value.GetType() ).c_str() ) );
					}					
				}

				g_eventSystem->FireEvent( eventName.GetAsString(), &args, EVERYWHERE );
			}
			break;

			case eOpCode::CHANGE_STATE:
			{
				ZephyrValue stateName = PopConstant();

				EventArgs args;
				args.SetValue( "entity", (void*)parentEntity );
				args.SetValue( "targetState", stateName.GetAsString() );
				g_eventSystem->FireEvent( "ChangeZephyrScriptState", &args, EVERYWHERE );
				
				// Bail out of this chunk to avoid trying to execute bytecode in the wrong update chunk
				return;
			}
			break;

			default:
			{
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
														ZephyrValueMap* globalVariables, 
														Entity* parentEntity )
{
	InterpretBytecodeChunk( bytecodeChunk, globalVariables, parentEntity, nullptr, nullptr );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
														ZephyrValueMap* globalVariables, 
														Entity* parentEntity, 
														EventArgs* eventArgs, 
														ZephyrValueMap* stateVariables )
{
	InterpretBytecodeChunk( bytecodeChunk, globalVariables, parentEntity, eventArgs, stateVariables );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::CopyEventArgVariables( EventArgs* eventArgs )
{
	if ( eventArgs == nullptr )
	{
		return;
	}

	// For each event arg that matches a known ZephyrType, add it to the local variables map so the event can use it
	std::map<std::string, TypedPropertyBase*> argKeyValuePairs = eventArgs->GetAllKeyValuePairs();

	for ( auto const& keyValuePair : argKeyValuePairs )
	{
		if ( keyValuePair.second->Is<float>() )
		{
			m_eventsVariablesCopy[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, 0.f ) );
		}
		else if ( keyValuePair.second->Is<int>() )
		{
			m_eventsVariablesCopy[keyValuePair.first] = ZephyrValue( (float)eventArgs->GetValue( keyValuePair.first, 0 ) );
		}
		else if ( keyValuePair.second->Is<double>() )
		{
			m_eventsVariablesCopy[keyValuePair.first] = ZephyrValue( (float)eventArgs->GetValue( keyValuePair.first, 0.0 ) );
		}
		else if ( keyValuePair.second->Is<bool>() )
		{
			m_eventsVariablesCopy[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, false ) );
		}
		else if ( keyValuePair.second->Is<Vec2>() )
		{
			m_eventsVariablesCopy[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, Vec2::ZERO ) );
		}
		else if ( keyValuePair.second->Is<std::string>() 
				  || keyValuePair.second->Is<char*>() )
		{
			m_eventsVariablesCopy[keyValuePair.first] = ZephyrValue( keyValuePair.second->GetAsString() );
		}

		// Any other variables will be ignored since they have no ZephyrType equivalent
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushConstant( const ZephyrValue& number )
{
	m_constantStack.push( number );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::PopConstant()
{
	GUARANTEE_OR_DIE( !m_constantStack.empty(), Stringf( "Constant stack is empty in script '%s'", m_parentEntity->GetScriptName().c_str() ) );
	
	ZephyrValue topConstant = m_constantStack.top();
	m_constantStack.pop();

	return topConstant;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::PeekConstant()
{
	GUARANTEE_OR_DIE( !m_constantStack.empty(), Stringf( "Constant stack is empty in script '%s'", m_parentEntity->GetScriptName().c_str() ) );

	return m_constantStack.top();
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushBinaryOp( const ZephyrValue& a, const ZephyrValue& b, eOpCode opCode )
{
	// Do type checking inside each sub function

	if ( a.GetType() == eValueType::NUMBER
		 && b.GetType() == eValueType::NUMBER )
	{
		PushNumberBinaryOp( a.GetAsNumber(), b.GetAsNumber(), opCode );
	}
	else if ( a.GetType() == eValueType::VEC2
			  && b.GetType() == eValueType::VEC2 )
	{
		PushVec2BinaryOp( a.GetAsVec2(), b.GetAsVec2(), opCode );
	}
	else if ( a.GetType() == eValueType::BOOL
			  && b.GetType() == eValueType::BOOL )
	{
		PushBoolBinaryOp( a.GetAsBool(), b.GetAsBool(), opCode );
	}
	else if ( a.GetType() == eValueType::STRING
			  && b.GetType() == eValueType::STRING )
	{
		PushStringBinaryOp( a.GetAsString(), b.GetAsString(), opCode );
	}
	else
	{
		ReportError( "Type mismatch" );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushNumberBinaryOp( NUMBER_TYPE a, NUMBER_TYPE b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::ADD:
		{
			NUMBER_TYPE result = a + b;
			PushConstant( result );
		}
		break;

		case eOpCode::SUBTRACT:
		{
			NUMBER_TYPE result = a - b;
			PushConstant( result );
		}
		break;

		case eOpCode::MULTIPLY:
		{
			NUMBER_TYPE result = a * b;
			PushConstant( result );
		}
		break;

		case eOpCode::DIVIDE:
		{
			NUMBER_TYPE result = a / b;
			PushConstant( result );
		}
		break;

		case eOpCode::NOT_EQUAL:
		{
			bool result = !IsNearlyEqual( a, b );
			PushConstant( result );
		}
		break;

		case eOpCode::EQUAL:
		{
			bool result = IsNearlyEqual( a, b );
			PushConstant( result );
		}
		break;

		case eOpCode::GREATER:
		{
			bool result = a > b;
			PushConstant( result );
		}
		break;

		case eOpCode::GREATER_EQUAL:
		{
			bool result = a >= b;
			PushConstant( result );
		}
		break;

		case eOpCode::LESS:
		{
			bool result = a < b;
			PushConstant( result );
		}
		break;

		case eOpCode::LESS_EQUAL:
		{
			bool result = a <= b;
			PushConstant( result );
		}
		break;

		default:
		{

		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushVec2BinaryOp( const Vec2& a, const Vec2& b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::ADD:
		{
			Vec2 result = a + b;
			PushConstant( result );
		}
		break;

		case eOpCode::SUBTRACT:
		{
			Vec2 result = a - b;
			PushConstant( result );
		}
		break;

		case eOpCode::MULTIPLY:
		{
			Vec2 result = a * b;
			PushConstant( result );
		}
		break;

		case eOpCode::NOT_EQUAL:
		{
			bool result = !IsNearlyEqual( a, b, .001f );
			PushConstant( result );
		}
		break;

		case eOpCode::EQUAL:
		{
			bool result = IsNearlyEqual( a, b, .001f );
			PushConstant( result );
		}
		break;

		default:
		{

		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushBoolBinaryOp( bool a, bool b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::NOT_EQUAL:
		{
			bool result = a != b;
			PushConstant( result );
		}
		break;

		case eOpCode::EQUAL:
		{
			bool result = a == b;
			PushConstant( result );
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushStringBinaryOp( const std::string& a, const std::string& b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::ADD:
		{
			std::string result = a;
			result.append( b );
			PushConstant( result );
		}
		break;

		case eOpCode::NOT_EQUAL:
		{
			bool result = a != b;
			PushConstant( result );
		}
		break;

		case eOpCode::EQUAL:
		{
			bool result = a == b;
			PushConstant( result );
		}
		break;

		default:
		{

		}
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::GetVariableValue( const std::string& variableName, const ZephyrValueMap& localVariables )
{
	// Try to find in local variables first
	auto localIter = localVariables.find( variableName );
	if ( localIter != localVariables.end() )
	{
		return localIter->second;
	}

	// Check event args
	if ( !m_eventsVariablesCopy.empty() )
	{
		auto eventIter = m_eventsVariablesCopy.find( variableName );
		if ( eventIter != m_eventsVariablesCopy.end() )
		{
			return eventIter->second;
		}
	}

	// Check state variables
	if ( m_stateVariables != nullptr )
	{
		auto stateIter = m_stateVariables->find( variableName );
		if ( stateIter != m_stateVariables->end() )
		{
			return stateIter->second;
		}
	}

	// Check global variables
	if ( m_globalVariables != nullptr )
	{
		auto globalIter = m_globalVariables->find( variableName );
		if ( globalIter != m_globalVariables->end() )
		{
			return globalIter->second;
		}
	}

	ReportError( Stringf( "Variable '%s' is undefined", variableName.c_str() ) );
	return ZephyrValue( ERROR_ZEPHYR_VAL );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::AssignToVariable( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap& localVariables )
{
	// Try to find in local variables first
	auto localIter = localVariables.find( variableName );
	if ( localIter != localVariables.end() )
	{
		localVariables[variableName] = value;
	}

	// Check state variables
	if ( m_stateVariables != nullptr )
	{
		auto stateIter = m_stateVariables->find( variableName );
		if ( stateIter != m_stateVariables->end() )
		{
			( *m_stateVariables )[variableName] = value;
		}
	}

	// Check global variables
	if ( m_globalVariables != nullptr )
	{
		auto globalIter = m_globalVariables->find( variableName );
		if ( globalIter != m_globalVariables->end() )
		{
			( *m_globalVariables )[variableName] = value;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// TODO: Find a more general way to set member variables
void ZephyrVirtualMachine::AssignToMemberVariable( const std::string& variableName, const std::string& memberName, const ZephyrValue& value, ZephyrValueMap& localVariables )
{
	// Try to find in local variables first
	auto localIter = localVariables.find( variableName );
	if ( localIter != localVariables.end() )
	{
		Vec2 vecValue = localIter->second.GetAsVec2();
		if		( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
		else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }

		localVariables[variableName] = ZephyrValue( vecValue );
	}

	// Check state variables
	if ( m_stateVariables != nullptr )
	{
		auto stateIter = m_stateVariables->find( variableName );
		if ( stateIter != m_stateVariables->end() )
		{
			Vec2 vecValue = stateIter->second.GetAsVec2();
			if ( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
			else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }

			( *m_stateVariables )[variableName] = ZephyrValue( vecValue );
		}
	}

	// Check global variables
	if ( m_globalVariables != nullptr )
	{
		auto globalIter = m_globalVariables->find( variableName );
		if ( globalIter != m_globalVariables->end() )
		{
			Vec2 vecValue = globalIter->second.GetAsVec2();
			if ( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
			else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }

			( *m_globalVariables )[variableName] = ZephyrValue( vecValue );
		}
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::GetGlobalVariableFromEntity( EntityId entityId, const std::string& variableName )
{
	Entity* entity = g_game->GetEntityById( entityId );
	if ( entity == nullptr )
	{
		ReportError( Stringf( "Unknown entity does not contain a member '%s'", variableName.c_str() ) );
		return ZephyrValue( ERROR_ZEPHYR_VAL );
	}

	return entity->GetGlobalVariable( variableName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::ReportError( const std::string& errorMsg )
{
	g_devConsole->PrintError( Stringf( "Error in script'%s': %s", m_parentEntity->GetScriptName().c_str(), errorMsg.c_str() ) );

	m_parentEntity->SetScriptObjectValidity( false );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVirtualMachine::IsErrorValue( const ZephyrValue& zephyrValue )
{
	return zephyrValue.GetAsEntity() == ERROR_ZEPHYR_VAL;
}

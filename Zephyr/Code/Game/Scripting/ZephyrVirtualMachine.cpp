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

	// Event variables don't need to be persisted after this call, so save a copy as local variables
	// TODO: Account for scopes inside if statements, etc.?
	std::map<std::string, ZephyrValue> localVariables;
	if ( bytecodeChunk.GetType() == eBytecodeChunkType::STATE 
		||  bytecodeChunk.GetType() == eBytecodeChunkType::EVENT )
	{
		localVariables = bytecodeChunk.GetVariables();

		if ( bytecodeChunk.GetType() == eBytecodeChunkType::EVENT )
		{
			CopyEventArgVariables( eventArgs, localVariables );
		}
	}

	int byteIdx = 0;
	while ( byteIdx < bytecodeChunk.GetNumBytes() )
	{
		// If this script has an error during interpretation, bail out to avoid running in a broken, unknown state
		if ( !parentEntity->IsScriptValid() )
		{
			return;
		}

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
			
			case eOpCode::ASSIGNMENT:
			{
				ZephyrValue variableName = PopConstant(); 
				ZephyrValue constantValue = PeekConstant();
				AssignToVariable( variableName.GetAsString(), constantValue, localVariables );
			}
			break;

			case eOpCode::MEMBER_ASSIGNMENT:
			{
				ZephyrValue constantValue = PopConstant();

				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor( localVariables );

				if ( IsErrorValue( memberAccessorResult.finalMemberVal ) )
				{
					return;
				}

				std::string lastMemberName = memberAccessorResult.memberNames.back();

				if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::ENTITY )
				{
					SetGlobalVariableInEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), lastMemberName, constantValue );
				}
				else if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::VEC2 )
				{
					if ( lastMemberName != "x"
						 && lastMemberName != "y" )
					{
						ReportError( Stringf( "'%s' is not a member of Vec2", lastMemberName.c_str() ) );
						return;
					}

					// This variable belongs to current entity, save like this to set local or state variables
					int memberCount = (int)memberAccessorResult.memberNames.size();
					if ( memberCount <= 1 )
					{
						AssignToMemberVariable( memberAccessorResult.baseObjName, lastMemberName, constantValue, localVariables );
					}
					else
					{
						// Account for this being a member of a different entity
						EntityId entityIdWithMember = memberAccessorResult.entityIdChain.back();
						std::string vec2VarName = memberAccessorResult.memberNames[memberCount - 2];

						SetGlobalVec2MemberVariableInEntity( entityIdWithMember, vec2VarName, lastMemberName, constantValue );
					}
				}

				PushConstant( constantValue );
			}
			break;

			case eOpCode::MEMBER_ACCESSOR:
			{
				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor( localVariables );

				if ( IsErrorValue( memberAccessorResult.finalMemberVal ) )
				{
					return;
				}

				// Push final member to top of constant stack
				const std::string& lastMemberName = memberAccessorResult.memberNames.back();
				int memberCount = (int)memberAccessorResult.memberNames.size();

				switch ( memberAccessorResult.finalMemberVal.GetType() )
				{
					case eValueType::BOOL:
					case eValueType::NUMBER:
					case eValueType::STRING:
					{
						ReportError( Stringf( "Variable of type %s can't have members. Tried to access '%s'",
											  ToString( memberAccessorResult.finalMemberVal.GetType() ).c_str(),
											  lastMemberName.c_str() ) );
						return;
					}
					break;

					case eValueType::VEC2:
					{
						if		( lastMemberName == "x" ) { PushConstant( memberAccessorResult.finalMemberVal.GetAsVec2().x ); }
						else if ( lastMemberName == "y" ) { PushConstant( memberAccessorResult.finalMemberVal.GetAsVec2().y ); }
						else
						{
							ReportError( Stringf( "'%s' is not a member of Vec2", lastMemberName.c_str() ) );
							return;
						}
					}
					break;

					case eValueType::ENTITY:
					{
						ZephyrValue val = GetGlobalVariableFromEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), lastMemberName );
						if ( IsErrorValue( val ) )
						{
							std::string entityVarName = memberCount > 1 ? memberAccessorResult.memberNames[memberCount - 2] : memberAccessorResult.baseObjName;

							ReportError( Stringf( "Variable '%s' is not a member of Entity '%s'", lastMemberName.c_str(), entityVarName.c_str() ) );
							return;
						}

						PushConstant( val );
					}
					break;
				}
			}
			break;

			case eOpCode::MEMBER_FUNCTION_CALL:
			{
				EventArgs args;
				args.SetValue( "entity", (void*)parentEntity );

				InsertParametersIntoEventArgs( args );

				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor( localVariables );

				if ( IsErrorValue( memberAccessorResult.finalMemberVal ) )
				{
					return;
				}

				int memberCount = (int)memberAccessorResult.memberNames.size();

				if ( memberAccessorResult.finalMemberVal.GetType() != eValueType::ENTITY )
				{
					std::string entityVarName = memberCount > 1 ? memberAccessorResult.memberNames[memberCount - 2] : memberAccessorResult.baseObjName;
					ReportError( Stringf( "Cannot call method on non entity variable '%s' with type '%s'", entityVarName.c_str(), ToString( memberAccessorResult.finalMemberVal.GetType() ).c_str() ) );
					return;
				}

				CallMemberFunctionOnEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), memberAccessorResult.memberNames.back(), &args );
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
				if ( !expression.EvaluateAsBool() )
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

				if ( !leftVal.EvaluateAsBool() 
					 || !rightVal.EvaluateAsBool() )
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

				if ( !leftVal.EvaluateAsBool()
					 && !rightVal.EvaluateAsBool() )
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
				GUARANTEE_OR_DIE( eventName.GetType() == eValueType::STRING, "Event name isn't a string" );

				EventArgs args;
				args.SetValue( "entity", (void*)parentEntity );

				InsertParametersIntoEventArgs( args );

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
void ZephyrVirtualMachine::CopyEventArgVariables( EventArgs* eventArgs, ZephyrValueMap& localVariables )
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
			localVariables[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, 0.f ) );
		}
		else if ( keyValuePair.second->Is<int>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( (float)eventArgs->GetValue( keyValuePair.first, 0 ) );
		}
		else if ( keyValuePair.second->Is<double>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( (float)eventArgs->GetValue( keyValuePair.first, 0.0 ) );
		}
		else if ( keyValuePair.second->Is<bool>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, false ) );
		}
		else if ( keyValuePair.second->Is<Vec2>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, Vec2::ZERO ) );
		}
		else if ( keyValuePair.second->Is<std::string>() 
				  || keyValuePair.second->Is<char*>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( keyValuePair.second->GetAsString() );
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
void ZephyrVirtualMachine::PushBinaryOp( ZephyrValue& a, ZephyrValue& b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::ADD:				PushAddOp( a, b ); break;
		case eOpCode::SUBTRACT:			PushSubtractOp( a, b ); break;
		case eOpCode::MULTIPLY:			PushMultiplyOp( a, b ); break;
		case eOpCode::DIVIDE:			PushDivideOp( a, b ); break;
		case eOpCode::NOT_EQUAL:		PushNotEqualOp( a, b ); break;
		case eOpCode::EQUAL:			PushEqualOp( a, b ); break;
		case eOpCode::GREATER:			PushGreaterOp( a, b ); break;
		case eOpCode::GREATER_EQUAL:	PushGreaterEqualOp( a, b ); break;
		case eOpCode::LESS:				PushLessOp( a, b ); break;
		case eOpCode::LESS_EQUAL:		PushLessEqualOp( a, b ); break;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushAddOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		NUMBER_TYPE result = a.GetAsNumber() + b.GetAsNumber();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		Vec2 result = a.GetAsVec2() + b.GetAsVec2();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::STRING
		 || bType == eValueType::STRING )
	{
		std::string result = a.EvaluateAsString();
		result.append( b.EvaluateAsString() );
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot add a variable of type %s with a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushSubtractOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		NUMBER_TYPE result = a.GetAsNumber() - b.GetAsNumber();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		Vec2 result = a.GetAsVec2() - b.GetAsVec2();
		PushConstant( result );
		return;
	}
	
	ReportError( Stringf( "Cannot subtract a variable of type %s from a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushMultiplyOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		NUMBER_TYPE result = a.GetAsNumber() * b.GetAsNumber();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		Vec2 result = a.GetAsVec2() * b.GetAsVec2();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::NUMBER )
	{
		Vec2 result = a.GetAsVec2() * b.GetAsNumber();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::NUMBER && bType == eValueType::VEC2 )
	{
		Vec2 result = a.GetAsNumber() * b.GetAsVec2();
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot multiply a variable of type %s by a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushDivideOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		if ( IsNearlyEqual( b.GetAsNumber(), 0.f, .0000001f ) )
		{
			ReportError( "Cannot divide a Number variable by 0" );
			return;
		}

		NUMBER_TYPE result = a.GetAsNumber() / b.GetAsNumber();
		PushConstant( result );
		return;
	}
	
	if ( aType == eValueType::VEC2 && bType == eValueType::NUMBER )
	{
		if ( IsNearlyEqual( b.GetAsNumber(), 0.f, .0000001f ) )
		{
			ReportError( "Cannot divide a Vec2 variable by 0" );
			return;
		}

		Vec2 result = a.GetAsVec2() / b.GetAsNumber();
		PushConstant( result );
		return;
	}
	
	ReportError( Stringf( "Cannot divide a variable of type %s by a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushNotEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = !IsNearlyEqual( a.GetAsNumber(), b.GetAsNumber(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		bool result = !IsNearlyEqual( a.GetAsVec2(), b.GetAsVec2(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::STRING && bType == eValueType::STRING )
	{
		bool result = a.GetAsString() != b.GetAsString();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::BOOL 
		 || bType == eValueType::BOOL )
	{
		bool result = a.EvaluateAsBool() != b.EvaluateAsBool();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::ENTITY && bType == eValueType::ENTITY )
	{
		bool result = a.GetAsEntity() != b.GetAsEntity();
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot compare a variable of type %s with a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = IsNearlyEqual( a.GetAsNumber(), b.GetAsNumber(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		bool result = IsNearlyEqual( a.GetAsVec2(), b.GetAsVec2(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::STRING && bType == eValueType::STRING )
	{
		bool result = a.GetAsString() == b.GetAsString();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::BOOL
		 || bType == eValueType::BOOL )
	{
		bool result = a.EvaluateAsBool() == b.EvaluateAsBool();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::ENTITY && bType == eValueType::ENTITY )
	{
		bool result = a.GetAsEntity() == b.GetAsEntity();
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot compare a variable of type %s with a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushGreaterOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = a.GetAsNumber() > b.GetAsNumber();
		PushConstant( result );
		return;
	}
	
	ReportError( Stringf( "Cannot check if a variable of type %s is greater than a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushGreaterEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = a.GetAsNumber() < b.GetAsNumber();
		PushConstant( !result );
		return;
	}

	ReportError( Stringf( "Cannot check if a variable of type %s is greater than or equal to a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushLessOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = a.GetAsNumber() < b.GetAsNumber();
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot check if a variable of type %s is less than a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushLessEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = a.GetAsNumber() > b.GetAsNumber();
		PushConstant( !result );
		return;
	}

	ReportError( Stringf( "Cannot check if a variable of type %s is less than a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
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
MemberAccessorResult ZephyrVirtualMachine::ProcessResultOfMemberAccessor( ZephyrValueMap localVariables )
{
	MemberAccessorResult memberAccessResult;

	ZephyrValue memberCountZephyr = PopConstant();
	int memberCount = (int)memberCountZephyr.GetAsNumber();

	ZephyrValue baseObjName = PopConstant();

	// Pull all members from the constant stack into a temp buffer for processing 
	// so in case there is a member error the stack is left in a good(ish) state 
	// Note: the members will be in reverse order, so account for that
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
		return memberAccessResult;
	}

	std::vector<EntityId> entityIdChain;

	// Process accessors excluding the final component, since that needs to be handled separately
	for ( int memberNameIdx = 0; memberNameIdx < (int)memberNames.size() - 1; ++memberNameIdx )
	{
		std::string& memberName = memberNames[memberNameIdx];

		switch ( memberVal.GetType() )
		{
			// This isn't the last member, so it can't be a primitive type
			case eValueType::BOOL:
			case eValueType::NUMBER:
			case eValueType::STRING:
			{
				ReportError( Stringf( "Variable of type %s can't have members. Tried to access '%s'",
									  ToString( memberVal.GetType() ).c_str(),
									  memberName.c_str() ) );
				return memberAccessResult;
			}
			break;

			case eValueType::VEC2:
			{
				if ( memberName == "x" ) { memberVal = ZephyrValue( memberVal.GetAsVec2().x ); }
				else if ( memberName == "y" ) { memberVal = ZephyrValue( memberVal.GetAsVec2().y ); }
				else
				{
					ReportError( Stringf( "'%s' is not a member of Vec2", memberName.c_str() ) );
					return memberAccessResult;
				}
			}
			break;

			case eValueType::ENTITY:
			{
				ZephyrValue val = GetGlobalVariableFromEntity( memberVal.GetAsEntity(), memberName );
				if ( IsErrorValue( val ) )
				{
					std::string entityVarName = memberNameIdx > 0 ? memberNames[memberNameIdx - 1] : baseObjName.GetAsString();

					ReportError( Stringf( "Variable '%s' is not a member of Entity '%s'", memberName.c_str(), entityVarName.c_str() ) );
					return memberAccessResult;
				}

				entityIdChain.push_back( memberVal.GetAsEntity() );
				memberVal = val;
			}
			break;
		}
	}

	memberAccessResult.finalMemberVal = memberVal;
	memberAccessResult.baseObjName = baseObjName.GetAsString();
	memberAccessResult.memberNames = memberNames;
	memberAccessResult.entityIdChain = entityIdChain;

	return memberAccessResult;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InsertParametersIntoEventArgs( EventArgs& args )
{
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
void ZephyrVirtualMachine::SetGlobalVariableInEntity( EntityId entityId, const std::string& variableName, const ZephyrValue& value )
{
	Entity* entity = g_game->GetEntityById( entityId );
	if ( entity == nullptr )
	{
		ReportError( Stringf( "Unknown entity does not contain a member '%s'", variableName.c_str() ) );
		return;
	}

	return entity->SetGlobalVariable( variableName, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::SetGlobalVec2MemberVariableInEntity( EntityId entityId, const std::string& variableName, const std::string& memberName, const ZephyrValue& value )
{
	Entity* entity = g_game->GetEntityById( entityId );
	if ( entity == nullptr )
	{
		ReportError( Stringf( "Unknown entity does not contain a member '%s'", variableName.c_str() ) );
		return;
	}

	return entity->SetGlobalVec2Variable( variableName, memberName, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::CallMemberFunctionOnEntity( EntityId entityId, const std::string& functionName, EventArgs* args )
{
	Entity* entity = g_game->GetEntityById( entityId );
	if ( entity == nullptr )
	{
		ReportError( Stringf( "Unknown entity does not contain a member '%s'", functionName.c_str() ) );
		return;
	}

	entity->FireScriptEvent( functionName, args );
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

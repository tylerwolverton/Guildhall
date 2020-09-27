#include "Game/Scripting/ZephyrVirtualMachine.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrVirtualMachine::ZephyrVirtualMachine()
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::Startup()
{

}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::Shutdown()
{

}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, ZephyrValueMap* globalVariables, Entity* parentEntity )
{
	ClearConstantStack();
	std::map<std::string, ZephyrValue> localVariables = *globalVariables;

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
				ZephyrValue constant = PopConstant();
				PushConstant( localVariables[constant.GetAsString()] );
			}
			break;

			case eOpCode::ASSIGNMENT:
			{
				ZephyrValue constant = PopConstant(); 
				localVariables[constant.GetAsString()] = PeekConstant();
			}
			break;

			case eOpCode::NEGATE:
			{
				ZephyrValue a = PopConstant();
				if ( a.GetType() == eValueType::NUMBER )
				{
					PushConstant( -a.GetAsNumber() );
				}
			}
			break;

			case eOpCode::ADD:
			case eOpCode::SUBTRACT:
			case eOpCode::MULTIPLY:
			case eOpCode::DIVIDE:
			{
				ZephyrValue b = PopConstant();
				ZephyrValue a = PopConstant();
				PushBinaryOp( a, b, opCode );
			}
			break;

			case eOpCode::FIRE_EVENT:
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
						case eValueType::STRING:	args.SetValue( param.GetAsString(), value.GetAsString() ); break;
					}					
				}

				g_eventSystem->FireEvent( eventName.GetAsString(), &args, EVERYWHERE );
			}
			break;

			default:
			{
			}
			break;
		}
	}

	UpdateGlobalVariables( *globalVariables, localVariables );
}

//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushConstant( const ZephyrValue& number )
{
	m_constantStack.push( number );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::PopConstant()
{
	ZephyrValue topConstant = m_constantStack.top();
	m_constantStack.pop();

	return topConstant;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::PeekConstant()
{
	return m_constantStack.top();
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushBinaryOp( const ZephyrValue& a, const ZephyrValue& b, eOpCode opCode )
{
	if ( a.GetType() == eValueType::NUMBER
		 && b.GetType() == eValueType::NUMBER )
	{
		PushNumberBinaryOp( a.GetAsNumber(), b.GetAsNumber(), opCode );
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

		default:
		{

		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::UpdateGlobalVariables( ZephyrValueMap& globalVariables, const ZephyrValueMap& localVariables )
{
	for ( auto& globalVarEntry : globalVariables )
	{
		auto localVarIter = localVariables.find( globalVarEntry.first );
		globalVariables[globalVarEntry.first] = localVarIter->second;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::ClearConstantStack()
{
	while ( !m_constantStack.empty() )
	{
		m_constantStack.pop();
	}
}

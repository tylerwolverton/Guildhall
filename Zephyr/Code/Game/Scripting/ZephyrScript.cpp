#include "Game/Scripting/ZephyrScript.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Game/Scripting/ZephyrScriptDefinition.hpp"
#include "Game/Scripting/ZephyrVirtualMachine.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrScript::ZephyrScript( const ZephyrScriptDefinition& scriptDef, Entity* parentEntity )
	: m_name( scriptDef.m_name )
	, m_scriptDef( scriptDef )
	, m_parentEntity( parentEntity )
{
	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	m_globalBytecodeChunk = new ZephyrBytecodeChunk( *m_scriptDef.GetGlobalBytecodeChunk() );
	GUARANTEE_OR_DIE( m_globalBytecodeChunk != nullptr, "Global Bytecode Chunk was null" );

	g_zephyrVM->InterpretStateBytecodeChunk( *m_globalBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );

	m_curStateBytecodeChunk = m_scriptDef.GetFirstStateBytecodeChunk();
	m_stateBytecodeChunks = m_scriptDef.GetAllStateBytecodeChunks();

	RegisterScriptEvents( m_globalBytecodeChunk );
	RegisterScriptEvents( m_curStateBytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
ZephyrScript::~ZephyrScript()
{
	g_eventSystem->DeRegisterObject( this );

	PTR_SAFE_DELETE( m_globalBytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::Update()
{
	if ( !m_scriptDef.IsValid() )
	{
		EventArgs args;
		args.SetValue( "entity", (void*)m_parentEntity );
		args.SetValue( "text", "Error Script" );
		args.SetValue( "color", Rgba8::RED );

		g_eventSystem->FireEvent( "PrintDebugText", &args );
		return;
	}

	if ( m_curStateBytecodeChunk != nullptr )
	{
		g_zephyrVM->InterpretStateBytecodeChunk( *m_curStateBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::UnloadScript()
{
	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	UnRegisterScriptEvents( m_curStateBytecodeChunk );
	UnRegisterScriptEvents( m_scriptDef.GetGlobalBytecodeChunk() );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::FireEvent( const std::string& eventName, EventArgs* args )
{
	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( eventName );
	if ( eventChunk != nullptr )
	{
		ZephyrValueMap* stateVariables = nullptr;
		if ( m_curStateBytecodeChunk != nullptr )
		{
			stateVariables = m_curStateBytecodeChunk->GetUpdateableVariables();
		}

		g_zephyrVM->InterpretEventBytecodeChunk( *eventChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, args, stateVariables );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::FireSpawnEvent()
{
	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( "Spawn" );
	if ( eventChunk != nullptr )
	{
		g_zephyrVM->InterpretEventBytecodeChunk( *eventChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::FireDieEvent()
{
	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( "Die" );
	if ( eventChunk != nullptr )
	{
		g_zephyrVM->InterpretEventBytecodeChunk( *eventChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::ChangeState( const std::string& targetState )
{
	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* targetStateBytecodeChunk = GetStateBytecodeChunk( targetState );
	if ( targetStateBytecodeChunk == nullptr )
	{
		// State doesn't exist, should be reported by compiler to avoid flooding with errors here
		return;
	}

	EventArgs args;
	FireEvent( "OnExit", &args );

	UnRegisterScriptEvents( m_curStateBytecodeChunk );

	m_curStateBytecodeChunk = targetStateBytecodeChunk;

	RegisterScriptEvents( m_curStateBytecodeChunk );

	FireEvent( "OnEnter", &args );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::InitializeGlobalVariables( const ZephyrValueMap& intialValues )
{
	if ( m_globalBytecodeChunk == nullptr )
	{
		return;
	}

	ZephyrValueMap* globalVariables = m_globalBytecodeChunk->GetUpdateableVariables();
	if ( globalVariables == nullptr )
	{
		return;
	}

	for ( auto initialValue : intialValues )
	{
		auto globalVarIter = globalVariables->find( initialValue.first );
		if ( globalVarIter == globalVariables->end() )
		{
			g_devConsole->PrintError( Stringf( "Cannot initialize nonexistent variable '%s' in script '%s'", initialValue.first.c_str(), m_name.c_str() ) );
			continue;
		}

		(*globalVariables)[initialValue.first] = initialValue.second;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::RegisterScriptEvents( ZephyrBytecodeChunk* bytecodeChunk )
{
	if ( bytecodeChunk == nullptr )
	{
		return;
	}

	for ( auto chunk : bytecodeChunk->GetEventBytecodeChunks() )
	{
		g_eventSystem->RegisterMethodEvent( chunk.first, "", eUsageLocation::EVERYWHERE, this, &ZephyrScript::OnEvent );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::UnRegisterScriptEvents( ZephyrBytecodeChunk* bytecodeChunk )
{
	if ( bytecodeChunk == nullptr )
	{
		return;
	}

	for ( auto chunk : bytecodeChunk->GetEventBytecodeChunks() )
	{
		g_eventSystem->DeRegisterMethodEvent( chunk.first, this, &ZephyrScript::OnEvent );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::OnEvent( EventArgs* args )
{
	std::string eventName = args->GetValue( "eventName", "" );

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( eventName );
	if ( eventChunk != nullptr )
	{
		ZephyrValueMap* stateVariables = nullptr;
		if ( m_curStateBytecodeChunk != nullptr )
		{
			stateVariables = m_curStateBytecodeChunk->GetUpdateableVariables();
		}

		g_zephyrVM->InterpretEventBytecodeChunk( *eventChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, args, stateVariables );
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScript::GetStateBytecodeChunk( const std::string& stateName )
{
	ZephyrBytecodeChunkMap::const_iterator  mapIter = m_stateBytecodeChunks.find( stateName );

	if ( mapIter == m_stateBytecodeChunks.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScript::GetEventBytecodeChunk( const std::string& eventName )
{
	if ( m_curStateBytecodeChunk != nullptr )
	{
		ZephyrBytecodeChunkMap::const_iterator  mapIter = m_curStateBytecodeChunk->GetEventBytecodeChunks().find( eventName );

		if ( mapIter != m_curStateBytecodeChunk->GetEventBytecodeChunks().cend() )
		{
			return mapIter->second;
		}
	}

	ZephyrBytecodeChunkMap::const_iterator mapIter = m_globalBytecodeChunk->GetEventBytecodeChunks().find( eventName );

	if ( mapIter != m_globalBytecodeChunk->GetEventBytecodeChunks().cend() )
	{
		return mapIter->second;
	}

	return nullptr;
}

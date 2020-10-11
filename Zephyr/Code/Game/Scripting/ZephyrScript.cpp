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

	ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
	GUARANTEE_OR_DIE( globalBytecodeChunk != nullptr, "Global Bytecode Chunk was null" );

	g_zephyrVM->InterpretBytecodeChunk( *globalBytecodeChunk, globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );

	m_curStateBytecodeChunk = m_scriptDef.GetFirstStateBytecodeChunk();
	m_stateBytecodeChunks = m_scriptDef.GetAllStateBytecodeChunks();

	RegisterScriptEvents( globalBytecodeChunk );
	RegisterScriptEvents( m_curStateBytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
ZephyrScript::~ZephyrScript()
{
	g_eventSystem->DeRegisterObject( this );
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
		ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
		g_zephyrVM->InterpretBytecodeChunk( *m_curStateBytecodeChunk, globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
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
void ZephyrScript::FireSpawnEvent()
{
	if ( !m_scriptDef.IsValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( "Spawn" );
	if ( eventChunk != nullptr )
	{
		ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
		g_zephyrVM->InterpretBytecodeChunk( *eventChunk, globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
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
		ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
		g_zephyrVM->InterpretBytecodeChunk( *eventChunk, globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
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

	UnRegisterScriptEvents( m_curStateBytecodeChunk );

	m_curStateBytecodeChunk = targetStateBytecodeChunk;

	RegisterScriptEvents( m_curStateBytecodeChunk );
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
		ZephyrBytecodeChunk* curBytecodeChunk = m_curStateBytecodeChunk;
		if ( curBytecodeChunk == nullptr )
		{
			curBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
		}

		g_zephyrVM->InterpretBytecodeChunk( *eventChunk, curBytecodeChunk->GetUpdateableVariables(), m_parentEntity, args );
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

	ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
	ZephyrBytecodeChunkMap::const_iterator mapIter = globalBytecodeChunk->GetEventBytecodeChunks().find( eventName );

	if ( mapIter != globalBytecodeChunk->GetEventBytecodeChunks().cend() )
	{
		return mapIter->second;
	}

	return nullptr;
}

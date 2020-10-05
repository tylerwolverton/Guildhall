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
	ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
	GUARANTEE_OR_DIE( globalBytecodeChunk != nullptr, "Global Bytecode Chunk was null" );

	g_zephyrVM->InterpretBytecodeChunk( *globalBytecodeChunk, globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );

	m_curStateBytecodeChunk = m_scriptDef.GetFirstStateBytecodeChunk();
	m_eventBytecodeChunks = m_scriptDef.GetAllEventBytecodeChunks();

	RegisterScriptEvents();
}


//-----------------------------------------------------------------------------------------------
ZephyrScript::~ZephyrScript()
{
	g_eventSystem->DeRegisterObject( this );
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::Update()
{
	if ( m_curStateBytecodeChunk != nullptr )
	{
		ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
		g_zephyrVM->InterpretBytecodeChunk( *m_curStateBytecodeChunk, globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::FireSpawnEvent()
{
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
	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( "Die" );
	if ( eventChunk != nullptr )
	{
		ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
		g_zephyrVM->InterpretBytecodeChunk( *eventChunk, globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::RegisterScriptEvents()
{
	for ( auto chunk : m_eventBytecodeChunks )
	{
		g_eventSystem->RegisterMethodEvent( chunk.first, "", eUsageLocation::EVERYWHERE, this, &ZephyrScript::OnEvent );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::OnEvent( EventArgs* args )
{
	std::string eventName = args->GetValue( "eventName", "" );

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( eventName );
	if ( eventChunk != nullptr )
	{
		ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
		g_zephyrVM->InterpretBytecodeChunk( *eventChunk, globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, args );
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScript::GetEventBytecodeChunk( const std::string& eventName )
{
	ZephyrBytecodeChunkMap::const_iterator  mapIter = m_eventBytecodeChunks.find( eventName );

	if ( mapIter == m_eventBytecodeChunks.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}



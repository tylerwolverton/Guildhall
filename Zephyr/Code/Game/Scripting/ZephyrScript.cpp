#include "Game/Scripting/ZephyrScript.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
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

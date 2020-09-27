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
}


//-----------------------------------------------------------------------------------------------
void ZephyrScript::Update()
{
	// For now, just interpret the first chunk
	std::vector<ZephyrBytecodeChunk*> bytecodeChunks = m_scriptDef.GetBytecodeChunks();
	if ( bytecodeChunks.size() > 0 )
	{
		ZephyrBytecodeChunk* globalBytecodeChunk = m_scriptDef.GetGlobalBytecodeChunk();
		g_zephyrVM->InterpretBytecodeChunk( *bytecodeChunks[0], globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
	}

	if ( !m_hasPrinted )
	{
		g_devConsole->PrintString( Stringf( "Hello, my name is %s!", m_name.c_str() ), Rgba8::MAGENTA );

		m_hasPrinted = true;
	}
}

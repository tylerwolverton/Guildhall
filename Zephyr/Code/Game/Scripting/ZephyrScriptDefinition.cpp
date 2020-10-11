#include "Game/Scripting/ZephyrScriptDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"


//-----------------------------------------------------------------------------------------------
// Static Definitions
std::map< std::string, ZephyrScriptDefinition* > ZephyrScriptDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::ZephyrScriptDefinition( ZephyrBytecodeChunk* stateMachineBytecodeChunk, 
												const ZephyrBytecodeChunkMap& bytecodeChunks )
	: m_stateMachineBytecodeChunk( stateMachineBytecodeChunk )
	, m_bytecodeChunks( bytecodeChunks )
{

}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::~ZephyrScriptDefinition()
{
	PTR_MAP_SAFE_DELETE( m_bytecodeChunks );
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScriptDefinition::GetBytecodeChunkByName( const std::string& name ) const
{
	ZephyrBytecodeChunkMap::const_iterator  mapIter = m_bytecodeChunks.find( name );

	if ( mapIter == m_bytecodeChunks.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScriptDefinition::GetFirstStateBytecodeChunk() const
{
	if ( m_bytecodeChunks.empty() )
	{
		return nullptr;
	}

	for ( auto chunk : m_bytecodeChunks )
	{
		if ( chunk.second->GetType() == eBytecodeChunkType::STATE
			 && chunk.second->IsInitialState() )
		{
			return chunk.second;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunkMap ZephyrScriptDefinition::GetAllStateBytecodeChunks() const
{
	ZephyrBytecodeChunkMap stateChunks;

	for ( auto chunk : m_bytecodeChunks )
	{
		if ( chunk.second->GetType() == eBytecodeChunkType::STATE )
		{
			stateChunks[chunk.first] = chunk.second;
		}
	}

	return stateChunks;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunkMap ZephyrScriptDefinition::GetAllEventBytecodeChunks() const
{
	ZephyrBytecodeChunkMap eventChunks;

	for ( auto chunk : m_bytecodeChunks )
	{
		if ( chunk.second->GetType() == eBytecodeChunkType::EVENT )
		{
			eventChunks[chunk.first] = chunk.second;
		}
	}

	return eventChunks;
}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition* ZephyrScriptDefinition::GetZephyrScriptDefinitionByPath( const std::string& scriptPath )
{
	std::map< std::string, ZephyrScriptDefinition* >::const_iterator  mapIter = ZephyrScriptDefinition::s_definitions.find( scriptPath );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition* ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( const std::string& scriptName )
{
	std::string fullPath = "Data/Scripts/" + scriptName;

	return GetZephyrScriptDefinitionByPath( fullPath );
}

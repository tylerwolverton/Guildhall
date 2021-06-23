#include "Engine/ZephyrCore/ZephyrScriptDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/ZephyrCore/ZephyrBytecodeChunk.hpp"


//-----------------------------------------------------------------------------------------------
// Static Definitions
std::map< std::string, ZephyrScriptDefinition* > ZephyrScriptDefinition::s_definitions;
std::string ZephyrScriptDefinition::s_dataPathSuffix;


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::ZephyrScriptDefinition( ZephyrBytecodeChunk* stateMachineBytecodeChunk, 
												const ZephyrBytecodeChunkMap& bytecodeChunks )
	: m_stateMachineBytecodeChunk( stateMachineBytecodeChunk )
	, m_bytecodeChunks( bytecodeChunks )
{
	s_dataPathSuffix = g_gameConfigBlackboard.GetValue( std::string( "dataPathSuffix" ), "" );
}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::~ZephyrScriptDefinition()
{
	PTR_MAP_SAFE_DELETE( m_bytecodeChunks );
	PTR_SAFE_DELETE( m_stateMachineBytecodeChunk );
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
	std::string fullPath = Stringf( "Data/Scripts%s/%s", s_dataPathSuffix.c_str(), scriptName.c_str() );

	return GetZephyrScriptDefinitionByPath( fullPath );
}

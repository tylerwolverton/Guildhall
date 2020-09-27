#include "Game/Scripting/ZephyrScriptDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Scripting/ZephyrBytecodeChunk.hpp"


//-----------------------------------------------------------------------------------------------
// Static Definitions
std::map< std::string, ZephyrScriptDefinition* > ZephyrScriptDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::ZephyrScriptDefinition( ZephyrBytecodeChunk* stateMachineBytecodeChunk, 
												const std::vector<ZephyrBytecodeChunk*>& bytecodeChunks )
	: m_stateMachineBytecodeChunk( stateMachineBytecodeChunk )
	, m_bytecodeChunks( bytecodeChunks )
{

}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::~ZephyrScriptDefinition()
{
	PTR_VECTOR_SAFE_DELETE( m_bytecodeChunks );
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

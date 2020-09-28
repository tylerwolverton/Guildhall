#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <map>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition
{
public:
	ZephyrScriptDefinition( ZephyrBytecodeChunk* stateMachineBytecodeChunk, const ZephyrBytecodeChunkMap& bytecodeChunks );
	~ZephyrScriptDefinition();

	ZephyrBytecodeChunk* GetGlobalBytecodeChunk() const										{ return m_stateMachineBytecodeChunk; }
	ZephyrBytecodeChunk* GetBytecodeChunkByName( const std::string& name ) const;
	// TODO: IS this enough or do we need a variable in StateMachine for the initial state?
	ZephyrBytecodeChunk* GetFirstStateBytecodeChunk() const;
	ZephyrBytecodeChunkMap GetAllEventBytecodeChunks() const;

	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByPath( const std::string& scriptPath );
	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByName( const std::string& scriptName );

public:
	static std::map< std::string, ZephyrScriptDefinition* > s_definitions;

	// TEMP
	std::string m_name;

private:
	ZephyrBytecodeChunk* m_stateMachineBytecodeChunk = nullptr;
	ZephyrBytecodeChunkMap m_bytecodeChunks;
};

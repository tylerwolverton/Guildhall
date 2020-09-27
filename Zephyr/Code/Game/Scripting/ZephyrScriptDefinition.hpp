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
	ZephyrScriptDefinition( ZephyrBytecodeChunk* stateMachineBytecodeChunk, const std::vector<ZephyrBytecodeChunk*>& bytecodeChunks );
	~ZephyrScriptDefinition();

	ZephyrBytecodeChunk*				GetGlobalBytecodeChunk() const				{ return m_stateMachineBytecodeChunk; }
	std::vector<ZephyrBytecodeChunk*>	GetBytecodeChunks() const					{ return m_bytecodeChunks; }

	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByPath( const std::string& scriptPath );
	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByName( const std::string& scriptName );

public:
	static std::map< std::string, ZephyrScriptDefinition* > s_definitions;

	// TEMP
	std::string m_name;

private:
	ZephyrBytecodeChunk* m_stateMachineBytecodeChunk = nullptr;
	std::vector<ZephyrBytecodeChunk*> m_bytecodeChunks;
};

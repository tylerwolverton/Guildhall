#pragma once
#include <map>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition
{
public:
	ZephyrScriptDefinition( const std::vector<ZephyrBytecodeChunk*>& bytecodeChunks );
	~ZephyrScriptDefinition();

	std::vector<ZephyrBytecodeChunk*> GetBytecodeChunks() const					{ return m_bytecodeChunks; }

	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByPath( const std::string& scriptPath );
	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByName( const std::string& scriptName );

public:
	static std::map< std::string, ZephyrScriptDefinition* > s_definitions;

	// TEMP
	std::string m_name;

private:
	std::vector<ZephyrBytecodeChunk*> m_bytecodeChunks;
};

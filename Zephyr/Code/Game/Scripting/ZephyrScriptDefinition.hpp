#pragma once
#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition
{
public:
	ZephyrScriptDefinition();

	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByPath( const std::string& scriptPath );
	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByName( const std::string& scriptName );

public:
	static std::map< std::string, ZephyrScriptDefinition* > s_definitions;

	// TEMP
	std::string m_name;
};

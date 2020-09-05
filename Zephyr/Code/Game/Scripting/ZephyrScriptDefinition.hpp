#pragma once
#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition
{
public:
	ZephyrScriptDefinition();

	static ZephyrScriptDefinition* GetZephyrScriptDefinition( const std::string& scriptName );

public:
	static std::map< std::string, ZephyrScriptDefinition* > s_definitions;
};

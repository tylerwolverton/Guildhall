#include "Game/Scripting/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
// Static Definitions
std::map< std::string, ZephyrScriptDefinition* > ZephyrScriptDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::ZephyrScriptDefinition()
{

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


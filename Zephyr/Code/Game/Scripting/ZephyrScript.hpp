#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;
class Entity;


//-----------------------------------------------------------------------------------------------
class ZephyrScript
{
public:
	ZephyrScript( const ZephyrScriptDefinition& scriptDef, Entity* parentEntity = nullptr );

	void Update();

private:
	std::string m_name;
	Entity* m_parentEntity = nullptr;

	ZephyrBytecodeChunk* m_curStateBytecodeChunk = nullptr;

	// TODO: save bytecode chunks into map that is managed by this class
	const ZephyrScriptDefinition& m_scriptDef;

	// std::map< "EventName", BytecodeChunk > m_eventChunks; 
	// std::map< "StateName", BytecodeChunk > m_stateUpdateChunks; 
};

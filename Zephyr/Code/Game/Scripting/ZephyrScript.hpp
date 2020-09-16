#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrScript
{
public:
	ZephyrScript( const ZephyrScriptDefinition& scriptDef );

	void Update();

private:
	std::string m_name;

	// TODO: save bytecode chunks into map that is managed by this class
	const ZephyrScriptDefinition& m_scriptDef;


	// TEMP
	bool m_hasPrinted = false;

	// BytecodeChunk m_updateChunk;
	// std::map< "EventName", BytecodeChunk > m_eventChunks; 
	// State(std::string StateName?) m_curState;
	// std::map< "StateName", BytecodeChunk > m_stateUpdateChunks; 
	// 
};

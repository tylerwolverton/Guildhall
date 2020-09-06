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

	// TEMP
	bool m_hasPrinted = false;

	// BytecodeChunk m_updateChunk;
	// std::map< "EventName", BytecodeChunk > m_eventChunks; 
	// State(std::string StateName?) m_curState;
	// std::map< "StateName", BytecodeChunk > m_stateUpdateChunks; 
	// 
};

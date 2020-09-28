#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Scripting/ZephyrCommon.hpp"

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
	~ZephyrScript();

	void Update();

	void FireSpawnEvent();
	void FireDieEvent();

private:
	void RegisterScriptEvents();

	void OnEvent( EventArgs* args );
	ZephyrBytecodeChunk* GetEventBytecodeChunk( const std::string& eventName );

private:
	std::string m_name;
	Entity* m_parentEntity = nullptr;

	ZephyrBytecodeChunk* m_curStateBytecodeChunk = nullptr;

	// TODO: save bytecode chunks into map that is managed by this class
	const ZephyrScriptDefinition& m_scriptDef;

	ZephyrBytecodeChunkMap m_eventBytecodeChunks; 
	// std::map< "StateName", BytecodeChunk > m_stateUpdateChunks; 
};

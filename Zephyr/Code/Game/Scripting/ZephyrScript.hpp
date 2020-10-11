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
	void UnloadScript();

	void FireSpawnEvent();
	void FireDieEvent();

	void ChangeState( const std::string& targetState );

private:
	void RegisterScriptEvents( ZephyrBytecodeChunk* bytecodeChunk );
	void UnRegisterScriptEvents( ZephyrBytecodeChunk* bytecodeChunk );

	void OnEvent( EventArgs* args );
	ZephyrBytecodeChunk* GetStateBytecodeChunk( const std::string& stateName );
	ZephyrBytecodeChunk* GetEventBytecodeChunk( const std::string& eventName );

private:
	std::string m_name;
	Entity* m_parentEntity = nullptr;

	ZephyrBytecodeChunk* m_curStateBytecodeChunk = nullptr;

	// TODO: save bytecode chunks into map that is managed by this class
	const ZephyrScriptDefinition& m_scriptDef;

	ZephyrBytecodeChunkMap m_stateBytecodeChunks; 
};

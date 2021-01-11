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

	void FireEvent( const std::string& eventName, EventArgs* args );

	void ChangeState( const std::string& targetState );

	void InitializeGlobalVariables( const ZephyrValueMap& intialValues );

private:
	void RegisterScriptEvents( ZephyrBytecodeChunk* bytecodeChunk );
	void UnRegisterScriptEvents( ZephyrBytecodeChunk* bytecodeChunk );

	void OnEvent( EventArgs* args );
	ZephyrBytecodeChunk* GetStateBytecodeChunk( const std::string& stateName );
	ZephyrBytecodeChunk* GetEventBytecodeChunk( const std::string& eventName );

private:
	std::string m_name;
	Entity* m_parentEntity = nullptr;

	const ZephyrScriptDefinition& m_scriptDef;
	ZephyrBytecodeChunk* m_globalBytecodeChunk = nullptr;
	ZephyrBytecodeChunk* m_curStateBytecodeChunk = nullptr;
	ZephyrBytecodeChunkMap m_stateBytecodeChunks; 
};
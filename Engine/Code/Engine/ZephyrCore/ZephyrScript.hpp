#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;
class ZephyrEntity;


//-----------------------------------------------------------------------------------------------
class ZephyrScript
{
public:
	ZephyrScript( const ZephyrScriptDefinition& scriptDef, ZephyrEntity* parentEntity = nullptr );
	~ZephyrScript();

	void Update();
	void UnloadScript();

	bool FireEvent( const std::string& eventName, EventArgs* args = nullptr );

	void ChangeState( const std::string& targetState );

	void InterpretGlobalBytecodeChunk();
	void InitializeGlobalVariables( const ZephyrValueMap& intialValues );
	void SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits );

	bool IsScriptValid() const;
	void SetScriptObjectValidity( bool isValid );

	std::string GetScriptName() const													{ return m_name; }
	ZephyrValue GetGlobalVariable( const std::string& varName );
	void		SetGlobalVariable( const std::string& varName, const ZephyrValue& value );
	//void		SetGlobalVec2Member( const std::string& varName, const std::string& memberName, const ZephyrValue& value );

	void InitializeEntityVariables();

	const ZephyrBytecodeChunk* GetBytecodeChunkByName( const std::string& chunkName ) const;

private:
	void OnEvent( EventArgs* args );
	ZephyrBytecodeChunk* GetStateBytecodeChunk( const std::string& stateName );
	ZephyrBytecodeChunk* GetEventBytecodeChunk( const std::string& eventName );

private:
	bool m_isScriptObjectValid = true;
	bool m_hasEnteredStartingState = false;

	std::string m_name;
	ZephyrEntity* m_parentEntity = nullptr;

	// Initial values for entity variables are given as names but must be translated into ids after all entities are loaded
	std::vector<EntityVariableInitializer> m_entityVarInits;

	const ZephyrScriptDefinition& m_scriptDef;
	ZephyrBytecodeChunk* m_globalBytecodeChunk = nullptr;
	ZephyrBytecodeChunk* m_curStateBytecodeChunk = nullptr;
	ZephyrBytecodeChunkMap m_stateBytecodeChunks; 
};

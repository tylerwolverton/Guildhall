#pragma once
#include "Engine/ZephyrCore/ZephyrCommon.hpp"

#include <map>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition
{
public:
	ZephyrScriptDefinition( ZephyrBytecodeChunk* stateMachineBytecodeChunk, const ZephyrBytecodeChunkMap& bytecodeChunks );
	~ZephyrScriptDefinition();

	bool IsValid() const																	{ return m_isValid; }
	void SetIsValid( bool isValid )															{ m_isValid = isValid; }

	ZephyrBytecodeChunk* GetGlobalBytecodeChunk() const										{ return m_stateMachineBytecodeChunk; }
	ZephyrBytecodeChunk* GetBytecodeChunkByName( const std::string& name ) const;
	ZephyrBytecodeChunk* GetFirstStateBytecodeChunk() const;
	ZephyrBytecodeChunkMap GetAllStateBytecodeChunks() const;
	ZephyrBytecodeChunkMap GetAllEventBytecodeChunks() const;

	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByPath( const std::string& scriptPath );
	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByName( const std::string& scriptName );

public:
	static std::map< std::string, ZephyrScriptDefinition* > s_definitions;

	// TEMP
	std::string m_name;

private:
	bool m_isValid = false;
	static std::string s_dataPathSuffix;

	ZephyrBytecodeChunk* m_stateMachineBytecodeChunk = nullptr;					// Owned by ZephyrScriptDefinition
	ZephyrBytecodeChunkMap m_bytecodeChunks;									// Owned by ZephyrScriptDefinition
};

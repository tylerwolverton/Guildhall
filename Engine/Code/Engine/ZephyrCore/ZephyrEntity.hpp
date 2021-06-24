#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrScript;
class ZephyrEntityDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrEntity
{	
public:
	ZephyrEntity( const ZephyrEntityDefinition& entityDef );
	virtual ~ZephyrEntity();

	virtual void				Update( float deltaSeconds );
	virtual void				Die();

	void						ChangeZephyrScriptState( const std::string& targetState );
	void						UnloadZephyrScript();
	void						ReloadZephyrScript();
	
	void						CreateZephyrScript( const ZephyrEntityDefinition& entityDef );
	void						InitializeZephyrEntityVariables();
	void						InitializeScriptValues( const ZephyrValueMap& initialValues );
	void						SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits );
	const ZephyrBytecodeChunk*	GetBytecodeChunkByName( const std::string& chunkName ) const;
	
	virtual ZephyrValue			GetGlobalVariable( const std::string& varName );
	virtual void				SetGlobalVariable( const std::string& varName, const ZephyrValue& value );
	//void						SetGlobalVec2Member( const std::string& varName, const std::string& memberName, const ZephyrValue& value );

	bool						IsScriptValid() const;
	void						SetScriptObjectValidity( bool isValid );
	std::string					GetScriptName() const;

	virtual const Vec2			GetPosition() const = 0;

	virtual EntityId			GetId() const											{ return m_id; }
	virtual std::string			GetName() const											{ return m_name; }
	virtual void				SetName( const std::string& name ) 						{ m_name = name; }

	void						FireSpawnEvent();
	bool						FireScriptEvent( const std::string& eventName, EventArgs* args );
	
	virtual bool				IsDead() const = 0;

	// Hook for adding in game specific args for each fired event
	virtual void				AddGameEventParams( EventArgs* args ) const = 0;

protected:
	const ZephyrEntityDefinition& m_entityDef;
	ZephyrScript*	m_scriptObj = nullptr;

	std::string		m_name;
	EntityId		m_id = -1;
	
	// Statics
	static EntityId	s_nextEntityId;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<ZephyrEntity*> ZephyrEntityVector;

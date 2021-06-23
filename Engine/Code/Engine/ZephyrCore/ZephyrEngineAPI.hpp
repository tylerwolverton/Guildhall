#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <unordered_set>
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrEntity;


//-----------------------------------------------------------------------------------------------
class ZephyrEngineAPI
{
public:
	ZephyrEngineAPI();
	virtual ~ZephyrEngineAPI();

	bool IsMethodRegistered( const std::string& methodName );

	virtual ZephyrEntity* GetEntityById( const EntityId& id ) const = 0;
	virtual ZephyrEntity* GetEntityByName( const std::string& name ) const = 0;

private:
	// Zephyr Script Events
	//void ChangeZephyrScriptState( EventArgs* args );

	//// Debug Events
	//void PrintDebugText( EventArgs* args );
	//void PrintDebugScreenText( EventArgs* args );
	//void PrintToConsole( EventArgs* args );

protected:
	std::unordered_set<std::string> m_registeredMethods;
};

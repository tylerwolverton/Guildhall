#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <unordered_set>
#include <string>


//-----------------------------------------------------------------------------------------------
class Entity;


//-----------------------------------------------------------------------------------------------
class GameAPI
{
public:
	GameAPI();
	~GameAPI();

	bool IsMethodRegistered( const std::string& methodName );

private:
	void UpdateEnemyCount( EventArgs* args );
	void DestroyEntity( EventArgs* args );

	void PrintDebugText( EventArgs* args );

private:
	std::unordered_set<std::string> m_registeredMethods;
};

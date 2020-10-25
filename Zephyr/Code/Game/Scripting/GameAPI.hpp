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
	// Debug Events
	void ChangeZephyrScriptState( EventArgs* args );
	void PrintDebugText( EventArgs* args );
	void PrintToConsole( EventArgs* args );

	// Game Events
	void DestroyEntity( EventArgs* args );
	void DamageEntity( EventArgs* args );
	void UpdateEnemyCount( EventArgs* args );
	void WinGame( EventArgs* args );

	// AI
	void MoveToLocation( EventArgs* args );
	void ChaseTargetEntity( EventArgs* args );
	void FleeTargetEntity( EventArgs* args );
	void GetEntityLocation( EventArgs* args );
	void GetNewWanderTargetPosition( EventArgs* args );
	void CheckForTarget( EventArgs* args );
	void GetDistanceToTarget( EventArgs* args );

	// Audio/Visual Effects
	void AddScreenShake( EventArgs* args );

private:
	std::unordered_set<std::string> m_registeredMethods;
};

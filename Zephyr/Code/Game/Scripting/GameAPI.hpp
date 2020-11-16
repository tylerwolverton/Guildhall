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
	void PrintDebugScreenText( EventArgs* args );
	void PrintToConsole( EventArgs* args );

	// Game Events
	void SpawnEntity( EventArgs* args );
	/**
	 * Destroys the entity who called this event.
	 */
	void DestroySelf( EventArgs* args );
	void DamageEntity( EventArgs* args );
	void StartDialogue( EventArgs* args );
	void EndDialogue( EventArgs* args );
	void AddLineOfDialogueText( EventArgs* args );
	void AddDialogueChoice( EventArgs* args );
	void StartInvincibility( EventArgs* args );
	void WinGame( EventArgs* args );

	// AI
	void MoveToLocation( EventArgs* args );
	void ChaseTargetEntity( EventArgs* args );
	void FleeTargetEntity( EventArgs* args );
	void GetEntityLocation( EventArgs* args );
	void GetNewWanderTargetPosition( EventArgs* args );
	void CheckForTarget( EventArgs* args );
	void GetDistanceToTarget( EventArgs* args );

	// Audio/Visual
	void ChangeSpriteAnimation( EventArgs* args );
	void PlaySound( EventArgs* args );
	void ChangeMusic( EventArgs* args );
	void AddScreenShake( EventArgs* args );

private:
	std::unordered_set<std::string> m_registeredMethods;
};

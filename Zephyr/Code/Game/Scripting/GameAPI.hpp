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
	// Zephyr Script Events
	void ChangeZephyrScriptState( EventArgs* args );

	// Debug Events
	void PrintDebugText( EventArgs* args );
	void PrintDebugScreenText( EventArgs* args );
	void PrintToConsole( EventArgs* args );

	// Game Events
	void SpawnEntity( EventArgs* args );
	void StartDialogue( EventArgs* args );
	void EndDialogue( EventArgs* args );
	void AddLineOfDialogueText( EventArgs* args );
	void AddDialogueChoice( EventArgs* args );
	void StartNewTimer( EventArgs* args );
	void WinGame( EventArgs* args );

	// Entity Events
	void DestroySelf( EventArgs* args );
	void DamageEntity( EventArgs* args );
	void ActivateInvincibility( EventArgs* args );
	void DeactivateInvincibility( EventArgs* args );
	void AddNewDamageTypeMultiplier( EventArgs* args );
	void ChangeDamageTypeMultiplier( EventArgs* args );
	void AddItemToInventory( EventArgs* args );
	void RemoveItemFromInventory( EventArgs* args );
	void CheckEntityForInventoryItem( EventArgs* args );
	void DisableCollisions( EventArgs* args );
	void EnableCollisions( EventArgs* args );

	// AI
	void MoveToLocation( EventArgs* args );
	void MoveInDirection( EventArgs* args );
	void ChaseTargetEntity( EventArgs* args );
	void FleeTargetEntity( EventArgs* args );
	void GetEntityLocation( EventArgs* args );
	void GetNewWanderTargetPosition( EventArgs* args );
	void CheckForTarget( EventArgs* args );
	void GetDistanceToTarget( EventArgs* args );

	// Input
	void RegisterKeyEvent( EventArgs* args );
	void UnRegisterKeyEvent( EventArgs* args );

	// Audio/Visual
	void ChangeSpriteAnimation( EventArgs* args );
	void PlaySound( EventArgs* args );
	void ChangeMusic( EventArgs* args );
	void AddScreenShake( EventArgs* args );

	// Map
	void SpawnEntitiesInRange( EventArgs* args );

private:
	Entity* GetItemEntityFromArgs( EventArgs* args );
	Entity* GetTargetEntityFromArgs( EventArgs* args );

private:
	std::unordered_set<std::string> m_registeredMethods;
};

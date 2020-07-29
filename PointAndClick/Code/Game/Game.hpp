#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Time/Timer.hpp"

#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class Entity;
class Actor;
class Cursor;
class DialogueState;
class Clock;
class Camera;
class Item;
class RandomNumberGenerator;
class TextBox;
class UIButton;
class UIPanel;
class UISystem;
class UIText;
class World;


//-----------------------------------------------------------------------------------------------
enum class eGameState
{
	LOADING,
	ATTRACT,
	PLAYING,
	DIALOGUE,
	GAME_OVER,
	VICTORY,
	PAUSED
};


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void Startup();
	void BeginFrame();
	void Update();
	void Render() const;
	void DebugRender() const;
	void EndFrame();
	void Shutdown();

	void RestartGame();
	
	eGameState	GetGameState() const													{ return m_gameState; }
	void		ChangeGameState( const eGameState& newGameState );
	void		ChangeMap( const std::string& mapName );

	bool		IsNoClipEnabled()														{ return m_isNoClipEnabled; }
	bool		IsDebugCameraEnabled()													{ return m_isDebugCameraEnabled; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }

	void		SetWorldCameraPosition( const Vec3& position );
	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

	void		AddItemToInventory( Entity* newItem );
	void		RemoveItemFromInventory( Entity* itemToRemove );
	void		RemoveItemFromInventory( const std::string& itemName );
	bool		IsItemInInventory( Entity* item );
	bool		IsItemInInventory( const std::string& itemName );
	
	void		SetPlayer( Actor* player )												{ m_player = player; }
	void		SetPlayerVerbState( eVerbState verbState );

	void		ClearCurrentActionText();
	void		SetNounText( const std::string& nounText );

	void		PrintTextOverPlayer( const std::string& text );
	void		PrintTextOverEntity( const Entity& entity, const std::string& text, float duration = 99999999.f );

	void		BeginConversation( DialogueState* initialDialogueState, Entity* dialoguePartner );
	void		ChangeDialogueState( DialogueState* newDialogueState );
	void		EndConversation();

	std::string GetNounText() const														{ return m_nounText; }

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void StopAllSounds();
	void ResetGame();

	void LoadAssets();
	void LoadDialogueStatesFromXml();
	void LoadMapsFromXml();
	void LoadEntitiesFromXml();

	void UpdateFromKeyboard();
	void UpdateMousePositions();
	void UpdateMouseWorldPosition();
	void UpdateMouseUIPosition();
	void UpdateCameras();
	void UpdateNPCResponse();

	void BuildMenus();
	void BuildHUD();
	void BuildVerbPanel();
	void BuildInventoryPanel();

	void AddDialogueOptionsToHUD( const std::vector<std::string>& dialogueChoices, float fontSize );

	// Button Events
	void OnMainMenuPlayButtonClicked( EventArgs* args );
	void OnMainMenuExitButtonClicked( EventArgs* args );
	void OnPauseMenuResumeButtonClicked( EventArgs* args );
	void OnPauseMenuExitButtonClicked( EventArgs* args );
	void OnTestButtonClicked( EventArgs* args );
	void OnVerbButtonClicked( EventArgs* args );
	void OnInventoryButtonClicked( EventArgs* args );
	void OnInventoryItemHoverStay( EventArgs* args );
	void OnTestButtonHoverBegin( EventArgs* args );
	void OnTestButtonHoverEnd( EventArgs* args );
	void OnMenuButtonHoverBegin( EventArgs* args );
	void OnMenuButtonHoverEnd( EventArgs* args );
	void OnDialogueChoiceClicked( EventArgs* args );
	void OnDialogueChoiceHoverBegin( EventArgs* args );
	void OnDialogueChoiceHoverEnd( EventArgs* args );

	void UpdateInventoryButtonImages();

	void SetInitialDialogueState( bool hasInitialDialogueHappened ) { m_hasInitialDialogueHappened = hasInitialDialogueHappened; }

private:
	Clock* m_gameClock = nullptr;

	Actor* m_player = nullptr;
	Cursor* m_cursor = nullptr;
	Entity* m_dialogueNPC = nullptr;
	std::string m_dialogueNPCText;

	// HUD
	UISystem* m_uiSystem = nullptr;

	UIPanel* m_dialoguePanel = nullptr;
	UIPanel* m_hudPanel = nullptr;
	UIPanel* m_verbPanel = nullptr;
	UIPanel* m_inventoryPanel = nullptr;
	UIPanel* m_currentActionPanel = nullptr;

	// Menus
	UIPanel* m_mainMenuPanel = nullptr;
	UIButton* m_mainMenuPlayButton = nullptr;
	UIButton* m_mainMenuExitButton = nullptr;
	UIPanel* m_pauseMenuPanel = nullptr;
	UIButton* m_pauseMenuResumeButton = nullptr;
	UIButton* m_pauseMenuExitButton = nullptr;
	UIPanel* m_victoryPanel = nullptr;
	UIButton* m_victoryRetryButton = nullptr;
	UIButton* m_victoryExitButton = nullptr;

	UIText* m_verbActionUIText = nullptr;
	std::string m_verbText;
	std::string m_nounText;
	std::string m_giveTargetNounText;

	UIButton* m_giveVerbButton = nullptr;
	UIButton* m_openVerbButton = nullptr;
	UIButton* m_closeVerbButton = nullptr;
	UIButton* m_pickUpVerbButton = nullptr;
	UIButton* m_talkToVerbButton = nullptr;

	std::vector<UIButton*> m_inventoryButtons;
	std::vector<Entity*> m_inventory;

	Timer m_dialogueTimer;
	DialogueState* m_curDialogueState = nullptr;

	bool m_hasInitialDialogueHappened = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_isFastMo = false;
	bool m_isDebugRendering = false;
	bool m_isNoClipEnabled = false;
	bool m_isDebugCameraEnabled = false;

	TextBox* m_debugInfoTextBox = nullptr;

	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	int m_loadingFrameNum = 0;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	Vec3 m_focalPoint = Vec3::ZERO;

	// Audio
	SoundPlaybackID m_attractMusicID = (SoundPlaybackID)-1;
	SoundPlaybackID m_gameplayMusicID = (SoundPlaybackID)-1;
	SoundPlaybackID m_victoryMusicID = (SoundPlaybackID)-1;

	eGameState m_gameState = eGameState::LOADING;

	World* m_world = nullptr;
	std::string m_curMap;
};

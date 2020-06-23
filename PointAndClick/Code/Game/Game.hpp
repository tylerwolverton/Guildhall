#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Time/Timer.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class Entity;
class Actor;
class RandomNumberGenerator;
class Clock;
class Camera;
class Item;
class World;
class TextBox;
class UIButton;
class UIPanel;
class UIText;


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
	
	void		LogMapDebugCommands(); 
	eGameState	GetGameState() const													{ return m_gameState; }
	void		ChangeGameState( const eGameState& newGameState );
	void		ChangeMap( const std::string& mapName );

	bool		IsNoClipEnabled()														{ return m_isNoClipEnabled; }
	bool		IsDebugCameraEnabled()													{ return m_isDebugCameraEnabled; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }

	void		SetWorldCameraPosition( const Vec3& position );
	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

	void		AddItemToInventory( Item* newItem );
	void		RemoveItemFromInventory( Item* itemToRemove );
	bool		IsItemInInventory( Item* item );
	
	void		SetPlayer( Actor* player )												{ m_player = player; }

	void		ClearCurrentActionText();
	void		SetNounText( const std::string& nounText )								{ m_nounText = nounText; }

	void		PrintTextOverPlayer( const std::string& text );
	void		PrintTextOverEntity( const Entity& entity, const std::string& text, float duration = 99999999.f );

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void StopAllSounds();

	void LoadAssets();
	void LoadMapsFromXml();
	void LoadActorsFromXml();
	void LoadItemsFromXml();

	void UpdateFromKeyboard();
	void LoadNewMap( const std::string& mapName );
	void UpdateMousePositions();
	void UpdateMouseWorldPosition();
	void UpdateMouseUIPosition();
	void UpdateCameras();

	void BuildHUD();
	void CleanupHUD();
	void BuildVerbPanel();
	void BuildInventoryPanel();

	void AddDialogueOptionsToHUD( const std::vector<std::string>& dialogueChoices, float fontSize );

	// Button Events
	void OnTestButtonClicked( EventArgs* args );
	void OnVerbButtonClicked( EventArgs* args );
	void OnInventoryItemHoverStay( EventArgs* args );
	void OnTestButtonHoverBegin( EventArgs* args );
	void OnTestButtonHoverEnd( EventArgs* args );
	void OnDialogueChoiceHoverBegin( EventArgs* args );
	void OnDialogueChoiceHoverEnd( EventArgs* args );

	void UpdateInventoryButtonImages();

private:
	Clock* m_gameClock = nullptr;

	Actor* m_player = nullptr;

	// HUD
	UIPanel* m_rootPanel = nullptr;
	UIPanel* m_dialoguePanel = nullptr;
	UIPanel* m_hudPanel = nullptr;
	UIPanel* m_verbPanel = nullptr;
	UIPanel* m_inventoryPanel = nullptr;
	UIPanel* m_currentActionPanel = nullptr;

	UIText* m_verbActionUIText = nullptr;
	std::string m_verbText;
	std::string m_nounText;

	UIButton* m_giveVerbButton = nullptr;
	UIButton* m_openVerbButton = nullptr;
	UIButton* m_closeVerbButton = nullptr;
	UIButton* m_pickUpVerbButton = nullptr;
	UIButton* m_talkToVerbButton = nullptr;

	std::vector<UIButton*> m_inventoryButtons;
	std::vector<Item*> m_inventory;

	Timer m_dialogueTimer;

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

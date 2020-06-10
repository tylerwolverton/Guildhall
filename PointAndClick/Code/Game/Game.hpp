#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class Entity;
class RandomNumberGenerator;
class Clock;
class Camera;
class Item;
class World;
class TextBox;
class UIButton;
class UIPanel;


//-----------------------------------------------------------------------------------------------
enum class eGameState
{
	LOADING,
	ATTRACT,
	PLAYING,
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

	void		PickupAtMousePosition();

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
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

	// Button Events
	void OnTestButtonClicked( EventArgs* args );
	void OnTestButtonHoverBegin( EventArgs* args );
	void OnTestButtonHoverEnd( EventArgs* args );
	void OnGiveButtonClicked( EventArgs* args );
	void OnOpenButtonClicked( EventArgs* args );
	void OnCloseButtonClicked( EventArgs* args );
	void OnPickUpButtonClicked( EventArgs* args );
	void OnTalkToButtonClicked( EventArgs* args );

	void UpdateInventoryButtonImages();

private:
	Clock* m_gameClock = nullptr;

	// HUD
	UIPanel* m_rootPanel = nullptr;
	UIPanel* m_hudPanel = nullptr;
	UIPanel* m_verbPanel = nullptr;
	UIPanel* m_inventoryPanel = nullptr;
	UIPanel* m_dialoguePanel = nullptr;

	UIButton* m_giveVerbButton = nullptr;
	UIButton* m_openVerbButton = nullptr;
	UIButton* m_closeVerbButton = nullptr;
	UIButton* m_pickUpVerbButton = nullptr;
	UIButton* m_talkToVerbButton = nullptr;

	std::vector<UIButton*> m_inventoryButtons;
	std::vector<Item*> m_inventory;

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

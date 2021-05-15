#pragma once
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Time/Timer.hpp"

#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class Actor;
class BitmapFont;
class Entity;
class Map;
class Material;
class RandomNumberGenerator;
class Clock;
class Camera;
class Item;
class TextBox;
class UISystem;
class UIElement;
class World;
enum class eItemType : int;


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
struct GameTimer
{
public:
	Timer timer;
	EntityId targetId = -1;
	std::string name;
	std::string callbackName;
	EventArgs* callbackArgs = nullptr;

public:
	GameTimer( Clock* clock, const EntityId& targetId = -1, const std::string& callbackName = "", const std::string& name = "", EventArgs* callbackArgsIn = nullptr );
	~GameTimer();
};


//-----------------------------------------------------------------------------------------------
enum class eCameraMode
{
	LOCKED_TO_PLAYER,
	FREE_MOVEMENT,
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
	void EndFrame();
	void Shutdown();

	void RestartGame();
	
	void		ChangeGameState( const eGameState& newGameState );
	eGameState  GetGameState() { return m_gameState; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }
	eCameraMode	GetCameraMode()															{ return m_cameraMode; }
	const Vec2	GetWorldCameraFocalPoint2D()											{ return m_focalPoint.XY(); }

	void		SetWorldCameraPosition( const Vec3& position );
	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

	void		WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );

	float		GetLastDeltaSecondsf();

	Entity*		GetEntityById( EntityId id );
	Entity*		GetEntityByName( const std::string& name );
	Map*		GetMapByName( const std::string& name );
	Map*		GetCurrentMap();
	void		SaveEntityByName( Entity* entity );
	
	void		PlaySoundByName( const std::string& soundName, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
	void		ChangeMusic( const std::string& musicName, bool isLooped = true, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
	void		ChangeMusicVolume( float newVolume );

	void		StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void		StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	float		GetTimerRemaining( const std::string& timerName );

	Entity*		GetPlayer() const { return m_player; }
	UISystem*	GetUISystem()															{ return m_uiSystem; }

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void LoadAssets();
	void LoadSounds();
	void LoadTileMaterialsFromXml();
	void LoadTilesFromXml();
	void LoadMapsFromXml();
	void LoadEntitiesFromXml();
	void LoadUIElementsFromXml();
	void LoadLevelThresholdElementsFromXml();
	void LoadWorldDefinitionFromXml();
	void LoadAndCompileZephyrScripts();
	void ReloadGame();
	void ReloadScripts();

	void UpdateFromKeyboard();
	void UpdateFromKeyboardPlaying();
	void UpdateFromKeyboardAttract();
	void UpdateFromKeyboardPaused();
	void UpdateFromKeyboardGameOver();
	void UpdateFromKeyboardVictory();
	void ChangeMap( const std::string& mapName );
	void LoadStartingMap( const std::string& mapName );
	void UpdateMousePositions();
	void UpdateMouseWorldPosition();
	void UpdateMouseUIPosition();
	void UpdateCameras();
	void UpdateTimers();

	void InitializeUIElements();
	void InitializeEquipmentUISlot( const std::string& uiElemName, eItemType itemType );

	void InitializeFPSHistory();
	void UpdateFramesPerSecond();
	float GetAverageFPS() const;
	void RenderFPSCounter() const;
	
	// Events
	void OnHUDHoverBegin( EventArgs* args );
	void OnHUDHoverEnd( EventArgs* args );
	void OnAttackClickEvent( EventArgs* args );
	void OnInventoryClickEvent( EventArgs* args );
	void OnInventoryReleaseEvent( EventArgs* args );
	void OnEquipmentClickEvent( EventArgs* args );
	void OnEquipmentReleaseEvent( EventArgs* args );

	bool SwapDragItemWithTargetSlot( UIElement* targetSlot );
	void AddItemToInventorySlot( Item* item, UIElement* targetSlot );
	Item* RemoveInventoryItemFromSlot( UIElement* targetSlot );

	void PrintBytecodeChunk( EventArgs* args );

private:
	std::string m_gameName = "Protogame2D";
	Clock* m_gameClock = nullptr;
	float m_fpsHistory[FRAME_HISTORY_COUNT];
	int m_fpsNextIdx = 0;
	float m_fpsHistorySum = 0.f;

	bool m_isPaused = false;
	bool m_isDebugRendering = false;

	int m_numDeaths = 0;

	// HUD
	UISystem* m_uiSystem = nullptr;

	Item* m_curInventoryDragItem;
	int m_sourceInventorySlotId = -1;

	TextBox* m_debugInfoTextBox = nullptr;

	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	BitmapFont* m_uiFont = nullptr;
	Material* m_uiFontMaterial = nullptr;

	float m_screenShakeIntensity = 0.f;

	// Cameras
	Camera*		m_worldCamera = nullptr;
	Camera*		m_uiCamera = nullptr;
	Vec3		m_focalPoint = Vec3::ZERO;
	eCameraMode m_cameraMode = eCameraMode::FREE_MOVEMENT;
	float		m_cameraSpeed = 1.f;

	// Default map data
	std::string m_dataPathSuffix;
	std::string m_defaultTileName;
	std::string m_defaultTileMaterialName;

	// Audio
	std::string m_curMusicName;
	SoundPlaybackID m_curMusicId = (SoundPlaybackID)-1;
	std::map<std::string, SoundID> m_loadedSoundIds;
	float m_maxMasterVolume = 1.f;

	// Game state
	eGameState m_gameState = eGameState::LOADING;
	int m_loadingFrameNum = 0;
	bool m_isInventoryOpen = false;

	Entity* m_player = nullptr;

	World* m_world = nullptr;
	std::string m_curMapName;
	std::string m_startingMapName;

	// Timer management
	std::vector<GameTimer*> m_timerPool;
};

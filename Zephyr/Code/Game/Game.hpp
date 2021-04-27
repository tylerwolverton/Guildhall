#pragma once
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Time/Timer.hpp"
#include "Engine/UI/UISystem.hpp"

#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class Actor;
class Entity;
class Map;
class RandomNumberGenerator;
class Clock;
class Camera;
class DialogueBox;
class TextBox;
class UISystem;
class UIPanel;
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
class Game
{
public:
	Game();
	~Game();

	void		Startup();
	void		BeginFrame();
	void		Update();
	void		Render() const;
	void		DebugRender() const;
	void		EndFrame();
	void		Shutdown();

	void		RestartGame();
	
	void		ChangeGameState( const eGameState& newGameState );
	eGameState  GetGameState()															{ return m_gameState; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }

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

	void		AddLineOfDialogueText( const std::string& text );
	void		AddDialogueChoice( const std::string& name, const std::string& text );
	void		SelectInDialogue( Entity* dialoguePartner );

	void		PlaySoundByName( const std::string& soundName, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
	void		ChangeMusic( const std::string& musicName, bool isLooped = true, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );

	void		StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void		StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void LoadAssets();
	void LoadSounds();
	void LoadTileMaterialsFromXml();
	void LoadTilesFromXml();
	void LoadMapsFromXml();
	void LoadEntitiesFromXml();
	void LoadWorldDefinitionFromXml();
	void LoadAndCompileZephyrScripts();
	void ReloadGame();
	void ReloadScripts();

	void UpdateFromKeyboard();
	void LoadStartingMap( const std::string& mapName );
	void UpdateMousePositions();
	void UpdateMouseWorldPosition();
	void UpdateMouseUIPosition();
	void UpdateCameras();
	void UpdateTimers();

	void InitializeFPSHistory();
	void InitializeUI();
	void UpdateFramesPerSecond();
	float GetAverageFPS() const;
	void RenderFPSCounter() const;

	// Events
	void PrintBytecodeChunk( EventArgs* args );

private:
	Clock* m_gameClock = nullptr;
	Timer* m_stepTimer = nullptr;
	float m_fpsHistory[FRAME_HISTORY_COUNT];
	int m_fpsNextIdx = 0;
	float m_fpsHistorySum = 0.f;

	bool m_isPaused = false;
	bool m_isDebugRendering = false;

	// HUD
	UISystem* m_uiSystem = nullptr;
	UIPanel* m_uiInfoPanel = nullptr;
	DialogueBox* m_dialogueBox;

	TextBox* m_debugInfoTextBox = nullptr;

	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	int m_loadingFrameNum = 0;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	Vec3 m_focalPoint = Vec3::ZERO;

	// Default map data
	std::string m_dataPathSuffix;
	std::string m_defaultTileName;
	std::string m_defaultTileMaterialName;

	// Audio
	std::string m_curMusicName;
	SoundPlaybackID m_curMusicId = (SoundPlaybackID)-1;

	std::map<std::string, SoundID> m_loadedSoundIds;

	eGameState m_gameState = eGameState::LOADING;

	Actor* m_player = nullptr;

	World* m_world = nullptr;
	std::string m_curMapName;
	std::string m_startingMapName;

	// Timer management
	std::vector<GameTimer*> m_timerPool;
};

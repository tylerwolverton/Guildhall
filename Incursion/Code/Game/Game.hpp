#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
struct Rgba8;
class BitmapFont;
class Clock;
class Entity;
class RandomNumberGenerator;
class Camera;
class World;


//-----------------------------------------------------------------------------------------------
enum GameState
{
	GAME_STATE_INVALID = -1,

	GAME_STATE_LOADING,
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_GAME_OVER,
	GAME_STATE_VICTORY,
	GAME_STATE_PAUSED,

	NUM_GAME_STATES
};


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void Startup();
	void BeginFrame() {}
	void Update();
	void Render() const;
	void DebugRender() const;
	void Shutdown();

	// Game state management 
	void StopAllMusic();
	void RestartGame();
	GameState GetGameState()													{ return m_gameState; }
	void ChangeGameState( GameState newGameState );

	// Debug commands
	bool IsNoClipEnabled()														{ return m_isNoClipEnabled; }
	bool IsDebugCameraEnabled()													{ return m_isDebugCameraEnabled; }

	// Camera
	void SetWorldCameraPosition( const Vec3& cameraPosition );
	void AddScreenShakeIntensity(float additionalIntensityFraction);

	void RenderDeathOverlay( float curDeathSeconds ) const;
	void RenderDeathOverlayFadeOut( float curDeathSeconds ) const;

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

	void RenderPauseOverlay() const;
	void RenderVictoryScreen() const;

	void LoadAssets();
	void LoadImages();
	void LoadAudio();
	void GenerateMaps();

	void PopulateFullScreenVertexes();

private:
	// Game state
	Clock* m_gameClock = nullptr;
	World* m_world = nullptr;
	GameState m_gameState = GAME_STATE_LOADING;
	int m_loadingFrameNum = 0;
	bool m_needToGenerateMaps = false;
	float m_curEndGameSeconds = END_GAME_TIMER_SECONDS;
	float m_curVictoryScreenSeconds = 0.f;

	// Debug options
	bool m_isSlowMo = false;
	bool m_isFastMo = false;
	bool m_isDebugRendering = false;
	bool m_isNoClipEnabled = false;
	bool m_isDebugCameraEnabled = false;

	// Audio
	SoundPlaybackID m_attractMusicID = (SoundPlaybackID)-1;
	SoundPlaybackID m_gameplayMusicID = (SoundPlaybackID)-1;
	SoundPlaybackID m_victoryMusicID = (SoundPlaybackID)-1;

	// Cameras
	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	Vec3 m_focalPoint = Vec3::ZERO;

	// Visual
	std::vector<Vertex_PCU> m_darkScreenVertexes;
	BitmapFont*				m_font = nullptr;
	float					m_screenShakeIntensity = 0.f;
};

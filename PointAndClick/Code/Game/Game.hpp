#pragma once
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
class World;
class TextBox;


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

	bool		IsNoClipEnabled()														{ return m_isNoClipEnabled; }
	bool		IsDebugCameraEnabled()													{ return m_isDebugCameraEnabled; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }

	void		SetWorldCameraPosition( const Vec3& position );
	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void LoadAssets();
	void LoadTilesFromXml();
	void LoadMapsFromXml();
	void LoadActorsFromXml();

	void UpdateFromKeyboard();
	void LoadNewMap( const std::string& mapName );
	void UpdateMousePositions();
	void UpdateMouseWorldPosition();
	void UpdateMouseUIPosition();
	void UpdateCameras();

private:
	Clock* m_gameClock = nullptr;
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

#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class Actor;
class Entity;
class RandomNumberGenerator;
class Clock;
class Camera;
class TextBox;
class UISystem;
class World;


//-----------------------------------------------------------------------------------------------
enum class eGameState
{
	ATTRACT,
	PLAYING,
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
	
	void		ChangeGameState( const eGameState& newGameState );

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }

	void		SetWorldCameraPosition( const Vec3& position );
	void		SetSceneCameras( const AABB2& sceneBounds );
	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

	// Commands
	void SaveConvexSceneToFile( EventArgs* args );
	void LoadConvexSceneFromFile( EventArgs* args );

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void LoadAssets();

	void UpdateFromKeyboard();
	void UpdateMousePositions();
	void UpdateMouseWorldPosition();
	void UpdateMouseUIPosition();
	void UpdateCameras();
	
	void InitializeFPSHistory();
	void UpdateFramesPerSecond();
	float GetAverageFPS() const;
	void RenderFPSCounter() const;
	void RenderStats() const;

private:
	Clock* m_gameClock = nullptr;
	float m_fpsHistory[FRAME_HISTORY_COUNT];
	int m_fpsNextIdx = 0;
	float m_fpsHistorySum = 0.f;

	bool m_isPaused = false;
	bool m_isDebugRendering = false;

	// HUD
	UISystem* m_uiSystem = nullptr;

	TextBox* m_debugInfoTextBox = nullptr;

	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	int m_loadingFrameNum = 0;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	Vec3 m_focalPoint = Vec3::ZERO;

	// Default map data
	std::string m_defaultTileName;
	std::string m_defaultTileMaterialName;
	
	eGameState m_gameState = eGameState::PLAYING;
	
	World* m_world = nullptr;
};

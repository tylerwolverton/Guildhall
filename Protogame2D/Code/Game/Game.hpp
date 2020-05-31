#pragma once
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
	GAME_STATE_LOADING,
	GAME_STATE_ATTRACT,
	GAME_STATE_PLAYING,
	GAME_STATE_GAME_OVER,
	GAME_STATE_VICTORY,
	GAME_STATE_PAUSED
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
	//void		SetWorldCameraOrthographicView( const AABB2& cameraBounds );
	//void		SetWorldCameraOrthographicView( const Vec2& bottomLeft, const Vec2& topRight );

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

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	Vec3 m_focalPoint = Vec3::ZERO;

	eGameState m_gameState = eGameState::GAME_STATE_LOADING;

	World* m_world = nullptr;
	std::string m_curMap;
};

#pragma once
#include "Engine/Math/Vec2.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class Entity;
class RandomNumberGenerator;
class Camera;
class World;
class TextBox;


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void		Startup();
	void		Update( float deltaSeconds );
	void		Render() const;
	void		DebugRender() const;
	void		Shutdown();

	void		RestartGame();
	
	void		SetWorldCameraOrthographicView( const AABB2& cameraBounds );
	void		SetWorldCameraOrthographicView( const Vec2& bottomLeft, const Vec2& topRight );

	bool		IsNoClipEnabled()														{ return m_isNoClipEnabled; }
	bool		IsDebugCameraEnabled()													{ return m_isDebugCameraEnabled; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }

	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );
	
public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void LoadAssets();

	void UpdateFromKeyboard( float deltaSeconds );
	void LoadNewMap( const std::string& mapName );
	void UpdateMousePositions( float deltaSeconds );
	void UpdateMouseWorldPosition( float deltaSeconds );
	void UpdateMouseUIPosition( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

private:
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

	World* m_world = nullptr;
	std::string m_curMap;
};

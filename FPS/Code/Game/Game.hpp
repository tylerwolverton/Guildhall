#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class Clock;
class Entity;
class RandomNumberGenerator;
class Camera;
class World;
class TextBox;
class Texture;
class GPUMesh;


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void		Startup();
	void		Update();
	void		Render() const;
	void		DebugRender() const;
	void		Shutdown();

	void		RestartGame();
	
	bool		IsNoClipEnabled()														{ return m_isNoClipEnabled; }
	bool		IsDebugCameraEnabled()													{ return m_isDebugCameraEnabled; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }

	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

	static bool SetMouseSensitivity( EventArgs* args );
	
public:
	RandomNumberGenerator* m_rng = nullptr;

	static float m_mouseSensitivityMultiplier;

private:
	void LoadAssets();

	void UpdateFromKeyboard();
	void LoadNewMap( const std::string& mapName );
	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );

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
	Texture* m_screenTexture;

	World* m_world = nullptr;
	std::string m_curMap;

	GPUMesh* m_cubeMesh = nullptr;
	Transform m_cubeMeshTransform;

	GPUMesh* m_planeMesh = nullptr;
	Transform m_planeMeshTransform;

	GPUMesh* m_sphereMesh = nullptr;
	std::vector<Transform> m_sphereMeshTransforms;
};

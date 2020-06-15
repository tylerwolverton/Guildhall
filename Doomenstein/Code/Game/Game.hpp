#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
class Clock;
class Entity;
class RandomNumberGenerator;
class Camera;
class GPUMesh;
class Material;
class TextBox;
class Texture;
class UIPanel;
class World;


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

	void		SetCameraPositionAndYaw( const Vec2& pos, float yaw );

	static bool SetMouseSensitivity( EventArgs* args );
	static bool SetAmbientLightColor( EventArgs* args );
	
public:
	RandomNumberGenerator* m_rng = nullptr;

	static float m_mouseSensitivityMultiplier;

private:
	void LoadAssets();
	void LoadXmlMapMaterials();
	void LoadXmlMapRegions();
	void LoadXmlMaps();
	void LoadNewMap( const std::string& mapName );

	void InitializeCameras();
	void InitializeMeshes();

	void BuildUIHud();

	void UpdateFromKeyboard();
	void UpdateCameraTransform( float deltaSeconds );
	void UpdateFramesPerSecond();

	void RenderDebugUI() const;
	void RenderFPSCounter() const;

	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );

	float GetAverageFPS() const;

private:
	Clock* m_gameClock = nullptr;
	float m_fpsHistory[FRAME_HISTORY_COUNT];

	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_isFastMo = false;
	bool m_isDebugRendering = false;
	bool m_isNoClipEnabled = false;
	bool m_isDebugCameraEnabled = false;

	// UI
	UIPanel* m_rootUIPanel = nullptr;
	UIPanel* m_hudUIPanel = nullptr;
	UIPanel* m_worldUIPanel = nullptr;

	TextBox* m_debugInfoTextBox = nullptr;

	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	World* m_world = nullptr;
	std::string m_curMapStr;

	// Meshes
	GPUMesh* m_cubeMesh = nullptr;
	GPUMesh* m_quadMesh = nullptr;
	GPUMesh* m_sphereMesh = nullptr;

	std::vector<Transform> m_cubeMeshTransforms;

	// Materials
	Material* m_testMaterial = nullptr;
		
	// Sounds
	SoundID m_testSound;

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 0.5f;
	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;
	float m_gamma = 2.2f;

	// Default map data
	std::string m_defaultMaterialStr;
	std::string m_defaultMapRegionStr;
};

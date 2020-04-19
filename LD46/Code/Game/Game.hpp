#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Material.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
class Clock;
class Entity;
class RandomNumberGenerator;
class Camera;
class GameObject;
class GPUMesh;
class Physics2D;
class TextBox;
class Texture;
class World;


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
	
	bool		IsNoClipEnabled()														{ return m_isNoClipEnabled; }
	bool		IsDebugCameraEnabled()													{ return m_isDebugCameraEnabled; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }

	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

	static bool SetMouseSensitivity( EventArgs* args );
	static bool SetAmbientLightColor( EventArgs* args );
	
public:
	RandomNumberGenerator* m_rng = nullptr;

	static float m_mouseSensitivityMultiplier;

private:
	void LoadAssets();
	void LoadNewMap( const std::string& mapName );

	void InitializeCameras();
	void InitializeMeshes();
	void InitializeMaterials();
	void InitializeLights();

	void UpdateFromKeyboard();
	void UpdateCameraTransform( float deltaSeconds );
	void UpdateDebugDrawCommands();
	void UpdateLights();

	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );

	void SpawnEnvironmentBox( const Vec3& location, const Vec3& dimensions, eSimulationMode simMode = SIMULATION_MODE_STATIC );
	void SpawnSwitch( const Vec3& location, const Vec3& orientation, const Vec3& dimensions );

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

	World* m_world = nullptr;
	std::string m_curMap;

	// Meshes
	GPUMesh* m_cubeMesh = nullptr;
	GPUMesh* m_quadMesh = nullptr;
	GPUMesh* m_sphereMesh = nullptr;

	Transform m_cubeMeshTransform;
		
	Physics2D* m_physics2D = nullptr;
	Rigidbody2D* m_playerRigidbody = nullptr;
	Rigidbody2D* m_cubeRigidbody = nullptr;

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 0.f;
	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;
	float m_gamma = 2.2f;

	Light m_lights[MAX_LIGHTS];

	float m_powerLevel = 1.f;

	float m_playerRadius = .25f;

	std::vector<Transform> m_wallTransforms;
	Transform m_floorTransform;

	std::vector<GameObject*> m_gameObjects;
	GameObject* m_player = nullptr;

	// Materials ( move to XML eventually )
	Material* m_wallMaterial;
	Material* m_floorMaterial;
	Material* m_ceilingMaterial;
	Material* m_whiteMaterial;
};

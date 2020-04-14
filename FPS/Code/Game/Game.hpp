#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
class Clock;
class Entity;
class RandomNumberGenerator;
class Camera;
class World;
class TextBox;
class Texture;
class GPUMesh;


//-----------------------------------------------------------------------------------------------
enum class eLightMovementMode
{
	STATIONARY,
	FOLLOW_CAMERA,
	LOOP
};


//-----------------------------------------------------------------------------------------------
enum class eLightType
{
	POINT,
	DIRECTIONAL,
	SPOT
};


//-----------------------------------------------------------------------------------------------
struct GameLight
{
	Light light;

	eLightMovementMode movementMode = eLightMovementMode::FOLLOW_CAMERA;
	eLightType type = eLightType::POINT;
	bool enabled = false;
};


//-----------------------------------------------------------------------------------------------
struct FresnelConstants
{
	Vec3 color;
	float power;
};


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
	static bool SetAmbientLightColor( EventArgs* args );
	static bool SetPointLightColor( EventArgs* args );
	
public:
	RandomNumberGenerator* m_rng = nullptr;

	static float m_mouseSensitivityMultiplier;

private:
	void LoadAssets();
	void LoadNewMap( const std::string& mapName );

	void UpdateFromKeyboard();
	void UpdateCameraTransform( float deltaSeconds );
	void UpdateDebugDrawCommands();
	void UpdateLightingCommands( float deltaSeconds );

	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );
	void UpdateLights();

	void PrintHotkeys();
	void ChangeShader( int nextShaderIdx );

	GameLight& GetCurGameLight()											{ return m_lights[m_currentLightIdx]; }
	Light& GetCurLight()													{ return m_lights[m_currentLightIdx].light; }

	void ChangeCurrentLightType( eLightType newLightype );
	std::string LightTypeToStr( eLightType lightType );

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

	GPUMesh* m_quadMesh = nullptr;
	Transform m_quadMeshTransform;

	GPUMesh* m_cubeMesh = nullptr;
	Transform m_cubeMeshTransform;
	
	GPUMesh* m_sphereMesh = nullptr;
	Transform m_sphereMeshTransform;

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = .1f;
	GameLight m_lights[MAX_LIGHTS];
	int m_currentLightIdx = 0;
	float m_specularFactor = 0.f;
	float m_specularPower = 1.f;
	float m_gamma = 2.2f;

	FresnelConstants m_fresnelData;
	float m_dissolveFactor = 0.f;

	std::vector<std::string> m_shaderPaths;
	std::vector<std::string> m_shaderNames;
	int m_currentShaderIdx = 0;
};

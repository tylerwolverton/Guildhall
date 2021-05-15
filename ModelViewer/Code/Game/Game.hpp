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
class RandomNumberGenerator;
class Camera;
class TextBox;
class Texture;
class CPUMesh;
class GPUMesh;
class Shader;
class Material;


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
	bool isEnabled = true;
};


//-----------------------------------------------------------------------------------------------
struct FresnelConstants
{
	Vec3 color;
	float power;
};


//-----------------------------------------------------------------------------------------------
struct DissolveConstants
{
	Vec3 startColor;
	float dissolveFactor;

	Vec3 endColor;
	float edgeWidth;
};


//-----------------------------------------------------------------------------------------------
struct ProjectionConstants
{
	Mat44 projectionMatrix;

	Vec3 position;
	float power;
};


//-----------------------------------------------------------------------------------------------
struct ColorTransformConstants
{
	Mat44 colorTransform;

	Vec3 tint;
	float tintPower;

	float transformPower;

	float padding0;
	float padding1;
	float padding2;
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
	
	// Events
	static bool SetMouseSensitivity( EventArgs* args );
	void		UnSubscribeGameMethods( EventArgs* args );
	void		SetAmbientLightColor( EventArgs* args );
	void		SetPointLightColor( EventArgs* args );

	void		LoadObjFile( EventArgs* args );
	void		LoadTWSMFile( EventArgs* args );
	void		SaveTWSMFile( EventArgs* args );
	
public:
	RandomNumberGenerator* m_rng = nullptr;

	static float m_mouseSensitivityMultiplier;

private:
	void LoadAssets();

	void InitializeMeshes();
	void InitializeLights();

	void UpdateFromKeyboard();
	void UpdateCameraTransform( float deltaSeconds );
	void UpdateLightingCommands( float deltaSeconds );
	void UpdateToggleCommands( float deltaSeconds );

	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );
	void UpdateLights();

	void PrintHotkeys();
	void PrintInfo();
	void ChangeShader( int nextShaderIdx );

	GameLight& GetCurGameLight()											{ return m_lights[m_currentLightIdx]; }
	Light& GetCurLight()													{ return m_lights[m_currentLightIdx].light; }

	void ChangeCurrentLightType( eLightType newLightype );
	std::string LightTypeToStr( eLightType lightType );

	void SetLightDirectionToCamera( Light& light );

private:
	Clock* m_gameClock = nullptr;

	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_isFastMo = false;
	bool m_isDebugRendering = false;
	bool m_isNoClipEnabled = false;
	bool m_isDebugCameraEnabled = false;
	
	// Render states
	bool m_diffuseOn = true;
	bool m_normalOn = true;
	bool m_specGlossEmitOn = true;
	bool m_diffuseLightingOn = true;
	bool m_specularHighlightingOn = true;
	bool m_emissiveGlowOn = true;
	bool m_wireframeOn = false;

	float m_diffuseEffect = 1.f;
	float m_specularEffect = 1.f;
	float m_emissiveEffect = 1.f;

	TextBox* m_debugInfoTextBox = nullptr;

	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	
	// Meshes
	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh* m_gpuMesh = nullptr;
	Transform m_meshTransform;
	
	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 0.f;
	GameLight m_lights[MAX_LIGHTS];
	int m_currentLightIdx = 0;
	float m_specularFactor = 0.f;
	float m_specularPower = 1.f;
	float m_gamma = 2.2f;
	float m_nearFogDist = 25.f;
	float m_farFogDist = 50.f;

	Mat44 m_projectionViewMatrix;

	ColorTransformConstants m_colorTransformConstants;

	std::vector<Shader*> m_shaders;
	int m_currentShaderIdx = 0;

	// Materials
	Material* m_meshMaterial = nullptr;

	Texture* m_testDiffuse = nullptr;
	Texture* m_testNormal = nullptr;
	Texture* m_testSpecGlossEmit = nullptr;
};

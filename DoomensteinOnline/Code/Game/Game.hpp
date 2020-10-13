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
class KeyButtonState;
class RandomNumberGenerator;
class Camera;
class GPUMesh;
class Material;
class TextBox;
class Texture;
class UIPanel;
class UISystem;
class World;


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	virtual void	Startup();
	virtual void	Update( const KeyButtonState* keyStates = nullptr, const Vec2& mouseDeltaPos = Vec2::ZERO );
	virtual void	Render() const;
	virtual void	DebugRender() const;
	virtual void	Shutdown();

	virtual void	RestartGame();
	
	const Vec2		GetMouseWorldPosition()														{ return m_mouseWorldPosition; }
	const Camera*	GetWorldCamera()															{ return m_worldCamera; }

	void			AddScreenShakeIntensity( float additionalIntensityFraction );

	void			PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

	void			SetCameraPositionAndYaw( const Vec2& pos, float yaw );
	
	void			WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );

	// Events
	static bool SetMouseSensitivity( EventArgs* args );
	static bool SetAmbientLightColor( EventArgs* args );
	
public:
	RandomNumberGenerator* m_rng = nullptr;
	bool g_raytraceFollowCamera = true;

	static float m_mouseSensitivityMultiplier;

private:
	void LoadAssets();
	void LoadXmlEntityTypes();
	void LoadXmlMapMaterials();
	void LoadXmlMapRegions();
	void LoadXmlMaps();
	void ChangeMap( const std::string& mapName );

	void InitializeCameras();

	void BuildUIHud();

	void UpdateFromKeyboard( const KeyButtonState* keyStates, const Vec2& mouseDeltaPos );
	void UpdateMovementFromKeyboard( const KeyButtonState* keyStates, const Vec2& mouseDeltaPos );
	void UpdateCameraTransformToMatchPlayer( const Vec2& mouseDeltaPos );
	void UpdateFramesPerSecond();

	void RenderDebugUI() const;
	void RenderFPSCounter() const;

	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );

	float GetAverageFPS() const;

	void PossesNearestEntity();
	
	// Events
	void WarpMapCommand( EventArgs* args );

private:
	Clock* m_gameClock = nullptr;
	float m_fpsHistory[FRAME_HISTORY_COUNT];

	Entity* m_player = nullptr;

	bool m_isPaused = false;
	bool m_isDebugRendering = false;

	// UI
	UISystem* m_uiSystem = nullptr;
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

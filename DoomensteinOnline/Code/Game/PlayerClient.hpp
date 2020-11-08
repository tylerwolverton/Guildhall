#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Game/Client.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
struct ClientRequest;
class Camera;
class Clock;
class Entity;
class KeyButtonState;
class UISystem;
class UIPanel;
class World;


//-----------------------------------------------------------------------------------------------
class PlayerClient : public Client
{
public:
	PlayerClient() = default;
	~PlayerClient() = default;

	virtual void Startup();
	virtual void Shutdown();

	virtual void BeginFrame();

	virtual void Update() override;
	virtual void Render( const World* gameWorld ) const override;

	const Vec2		GetMouseWorldPosition() { return m_mouseWorldPosition; }
	const Camera*	GetWorldCamera() { return m_worldCamera; }

	void			AddScreenShakeIntensity( float additionalIntensityFraction );
	
	void			SetCameraPositionAndYaw( const Vec2& pos, float yaw );

	virtual void SetClientId( int id ) override												{ m_clientId = id; }

	virtual void SetPlayer( Entity* playerEntity ) override									{ m_player = playerEntity; }

	// Events
	static bool SetMouseSensitivity( EventArgs* args );
	static bool SetAmbientLightColor( EventArgs* args );

private:
	std::vector<ClientRequest*> ProcessInputAndConvertToClientRequests();

	void InitializeCameras();

	void BuildUIHud();

	void UpdateCameraTransformToMatchPlayer( const Vec2& mouseDeltaPos );
	void UpdateFramesPerSecond();
	
	void DebugRender( const World* gameWorld ) const;
	void RenderDebugUI() const;
	void RenderFPSCounter() const;

	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );

	float GetAverageFPS() const;

public:
	static float m_mouseSensitivityMultiplier;
	bool g_raytraceFollowCamera = true;

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
	
	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 0.5f;
	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;
	float m_gamma = 2.2f;
};

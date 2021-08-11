#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Time/Timer.hpp"
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
class UISystem;
class Map;
class World;


//-----------------------------------------------------------------------------------------------
enum class eGameState
{
	LOADING,
	ATTRACT,
	PLAYING,
	GAME_OVER,
	VICTORY,
	PAUSED
};


//-----------------------------------------------------------------------------------------------
struct GameTimer
{
public:
	Timer timer;
	EntityId targetId = -1;
	std::string name;
	std::string callbackName;
	EventArgs* callbackArgs = nullptr;

public:
	GameTimer( Clock* clock, const EntityId& targetId = -1, const std::string& callbackName = "", const std::string& name = "", EventArgs* callbackArgsIn = nullptr );
	~GameTimer();
};


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void			Startup();
	void			Update();
	void			Render() const;
	void			DebugRender() const;
	void			Shutdown();

	void			RestartGame();
	
	const Vec2		GetMouseWorldPosition()														{ return m_mouseWorldPosition; }
	const Camera*	GetWorldCamera()															{ return m_worldCamera; }

	void			AddScreenShakeIntensity( float additionalIntensityFraction );
	
	void			SetCameraPositionAndYaw( const Vec2& pos, float yaw );
	
	void			WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );
	void			WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );

	Entity*			GetEntityById( EntityId id );
	Entity*			GetEntityByName( const std::string& name );
	Map*			GetMapByName( const std::string& name );
	Map*			GetCurrentMap();
	void			SaveEntityByName( Entity* entity );

	void			PlaySoundByName( const std::string& soundName, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
	void			ChangeMusic( const std::string& musicName, bool isLooped = true, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );

	void			StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void			StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );

	static bool		SetMouseSensitivity( EventArgs* args );
	static bool		SetAmbientLightColor( EventArgs* args );
	
public:
	RandomNumberGenerator* m_rng = nullptr;
	bool g_raytraceFollowCamera = true;

	static float m_mouseSensitivityMultiplier;

private:
	void LoadAssets();
	void LoadSounds();
	void LoadXmlUIElements();
	void LoadXmlEntityTypes();
	void LoadXmlMapMaterials();
	void LoadXmlMapRegions();
	void LoadXmlMaps();
	void LoadWorldDefinitionFromXml();
	void LoadAndCompileZephyrScripts();
	void ReloadGame();
	void ReloadScripts();

	void ChangeMap( const std::string& mapName );

	void InitializeCameras();

	void AddGunToUI();

	void UpdateFromKeyboard();
	void LoadStartingMap( const std::string& mapName );
	void UpdateMovementFromKeyboard();
	void UpdateCameraTransformToMatchPlayer();
	void UpdateTimers();
	void UpdateFramesPerSecond();

	void RenderDebugUI() const;
	void RenderFPSCounter() const;

	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );
	void SetLightDirectionToCamera( Light& light );

	float GetAverageFPS() const;

	void PossesNearestEntity();
	
	// Events
	void WarpMapCommand( EventArgs* args );

private:
	Clock* m_gameClock = nullptr;
	float m_fpsHistory[FRAME_HISTORY_COUNT];

	Entity* m_player = nullptr;

	eGameState m_gameState = eGameState::LOADING;
	bool m_isDebugRendering = false;

	// UI
	//UIPanel* m_rootUIPanel = nullptr;
	UISystem* m_uiSystem = nullptr;
	UIPanel* m_hudUIPanel = nullptr;
	UIPanel* m_worldUIPanel = nullptr;
	
	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	World* m_world = nullptr;
	std::string m_startingMapName;
			
	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 0.1f;
	// TODO: Turn into light pool
	//GameLight m_lights[MAX_LIGHTS];
	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;
	float m_gamma = 2.2f;

	// Default map data
	std::string m_defaultMaterialStr;
	std::string m_defaultMapRegionStr;
	std::string m_defaultMapRegionCollisionLayerStr;

	// Audio
	std::string m_curMusicName;
	SoundPlaybackID m_curMusicId = (SoundPlaybackID)-1;

	std::map<std::string, SoundID> m_loadedSoundIds;

	// Timer management
	std::vector<GameTimer*> m_timerPool;
};

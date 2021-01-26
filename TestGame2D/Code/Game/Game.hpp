#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"


//-----------------------------------------------------------------------------------------------
class Entity;
struct Rgba8;
class RandomNumberGenerator;
class Camera;
class World;
class Texture;
class SpriteSheet;
class SpriteAnimDefinition;
class SpriteDefinition;


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void Startup();
	void Update( float deltaSeconds );
	void Render() const;
	void DebugRender() const;
	void Shutdown();

	void RestartGame();
	
	void SetWorldCameraOrthographicView( const AABB2& cameraBounds );
	void SetWorldCameraOrthographicView( const Vec2& bottomLeft, const Vec2& topRight );

	bool IsNoClipEnabled()														{ return m_isNoClipEnabled; }
	bool IsDebugCameraEnabled()													{ return m_isDebugCameraEnabled; }

	void AddScreenShakeIntensity(float additionalIntensityFraction);

	static bool LogToDevConsole( EventArgs* args );

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void LoadAssets();

	void RenderTestSpriteAnimations() const;
	void RenderTestText() const;
	void RenderTestTextInBox() const;
	void RenderSquareTestSprite() const;
	void RenderNonSquareTestSprite() const;
	void RenderMousePointer() const;

	void RenderSpriteAnimation( const SpriteDefinition& spriteDef, const Vec2& position ) const;
	void RenderNonSquareTestAnims() const;

	void LoadTestXml();
	void LoadTestImage();

	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateCameras( float deltaSeconds );

private:
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_isFastMo = false;
	bool m_isDebugRendering = false;
	bool m_isNoClipEnabled = false;
	bool m_isDebugCameraEnabled = false;

	float m_screenShakeIntensity = 0.f;

	World* m_world = nullptr;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	// Test assets
	float					m_secondsSinceStart = -10.f;
	Texture*				m_spriteAtlas4x4Texture = nullptr;
	SpriteSheet*			m_spriteAtlas4x4SpriteSheet = nullptr;

	Texture*				m_spriteSheet8x2Texture = nullptr;
	SpriteSheet*			m_spriteSheet8x2SpriteSheet = nullptr;
	SpriteAnimDefinition*	m_spriteSheet8x2AnimDefLoop = nullptr;
	SpriteAnimDefinition*	m_spriteSheet8x2AnimDefOnce = nullptr;
	SpriteAnimDefinition*	m_spriteSheet8x2AnimDefPingPong = nullptr;
};

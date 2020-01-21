#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"


//-----------------------------------------------------------------------------------------------
class PlayerShip;
class Entity;
struct Rgba8;
class RandomNumberGenerator;
class Camera;


//-----------------------------------------------------------------------------------------------
enum GameState
{
	GAME_STATE_INVALID = -1,

	GAME_STATE_ATTRACT,
	GAME_STATE_PLAY,

	NUM_GAME_STATES
};


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

	void CheckCollisions();
	void RestartGame();
	
	// Input handling "wrappers" to alert PlayerShip of input events
	void AcceleratePlayerShip();
	void StopAcceleratingPlayerShip();

	void TurnPlayerShipLeft();
	void TurnPlayerShipRight();
	void StopTurningPlayerShipLeft();
	void StopTurningPlayerShipRight();

	void HandleBulletFired();
	void HandleStarburstBulletFired();
	void HandleBulletReload();

	// Spawn new entities
	void SpawnBullet( const Vec2& pos, const Vec2& orientation );
	void SpawnStarburstBullet( const Vec2& pos, const Vec2& orientation );
	void SpawnMultipleAsteroids( int numAsteroids );
	void SpawnAsteroid();
	void SpawnBeetles( int numBeetles );
	void SpawnWasps( int numWasps );
	void SpawnDebris( const Vec2& position, const Vec2& velocity, const Rgba8& color, int numPieces );
	
	void RespawnPlayerIfDead();

	Vec2 GetPlayerPosition() const;

	void AddScreenShakeIntensity(float additionalIntensityFraction);

	GameState GetGameState()				{ return m_curGameState; }
	void SetGameState(GameState gameState);

public:
	RandomNumberGenerator* m_randNumGen = nullptr;

private:
	void UpdateCameras( float deltaSeconds );

	void DeleteGarbageEntities();

	void UpdateEntityArray( Entity** entities, int entityCount, float deltaSeconds );
	void RenderEntityArray( Entity** entities, int entityCount ) const;
	void DebugRenderEntityArray( Entity** entities, int entityCount ) const;
	void DeleteGarbageEntitiesInArray( Entity** entities, int entityCount, int& curEntityCount ); // Pass entity count by non-const reference so the correct entity tpye count can be updated by the generic method

	void RenderAttractScreen() const;
	void DrawDebugLinesToPlayer() const;
	
	void StartWave( int waveNum );
	void CheckForWaveChange();

private:
	// Use Entity arrays so generic Update, Render, and DeleteGarbage methods can be used
	PlayerShip* m_playerShip = nullptr;
	Entity**	m_bullets = nullptr;
	Entity**	m_asteroids = nullptr;
	Entity**	m_beetles = nullptr;
	Entity**	m_wasps = nullptr;
	Entity**	m_debrisPieces = nullptr;

	int	m_curBulletCount = 0;
	int	m_curAsteroidCount = 0;
	int	m_curBeetleCount = 0;
	int	m_curWaspCount = 0;
	int	m_curDebrisPiecesCount = 0;

	bool m_isDebugRendering = false;

	float m_screenShakeIntensity = 0.f;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	int			m_curWave = 1;
	GameState	m_curGameState = GameState::GAME_STATE_ATTRACT;
	float		m_curGameRestartTimer = 0.f;

	// Attract mode logo
	Rgba8 m_logoColor = Rgba8( 45, 100, 200 );
	float m_logoRedChangeFactor = 1.f;
	float m_logoGreenChangeFactor = 1.f;
	float m_logoBlueChangeFactor = 1.f;
};

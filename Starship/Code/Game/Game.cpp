#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Game/GameCommon.hpp"

#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/StarburstBullet.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Beetle.hpp"
#include "Game/Wasp.hpp"
#include "Game/Debris.hpp"


//-----------------------------------------------------------------------------------------------
Game::Game()
{
} 


//-----------------------------------------------------------------------------------------------
Game::~Game()
{
}


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	m_worldCamera = new Camera();
	m_uiCamera = new Camera();

	m_playerShip = new PlayerShip( this, Vec2( WORLD_CENTER_X, WORLD_CENTER_Y ) );
	
	m_bullets = new Entity*[MAX_BULLETS];
	for ( int bulletIndex = 0; bulletIndex < MAX_BULLETS; ++bulletIndex )
	{
		m_bullets[bulletIndex] = nullptr;
	}

	m_asteroids = new Entity*[MAX_ASTEROIDS];
	for ( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; ++asteroidIndex )
	{
		m_asteroids[asteroidIndex] = nullptr;
	}

	m_beetles = new Entity*[MAX_BEETLES];
	for ( int beetleIndex = 0; beetleIndex < MAX_BEETLES; ++beetleIndex )
	{
		m_beetles[beetleIndex] = nullptr;
	}

	m_wasps = new Entity*[MAX_WASPS];
	for ( int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex )
	{
		m_wasps[waspIndex] = nullptr;
	}

	m_debrisPieces = new Entity*[MAX_DEBRIS_PIECES];
	for ( int debrisIndex = 0; debrisIndex < MAX_DEBRIS_PIECES; ++debrisIndex )
	{
		m_debrisPieces[debrisIndex] = nullptr;
	}

	m_randNumGen = new RandomNumberGenerator();

	m_curWave = 1;
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	delete m_randNumGen;
	m_randNumGen = nullptr;

	delete[] m_debrisPieces;
	m_debrisPieces = nullptr;
	m_curDebrisPiecesCount = 0;

	delete[] m_wasps;
	m_wasps = nullptr;
	m_curWaspCount = 0;

	delete[] m_beetles;
	m_beetles = nullptr;
	m_curBeetleCount = 0;

	delete[] m_asteroids;
	m_asteroids = nullptr;
	m_curAsteroidCount = 0;

	delete[] m_bullets;
	m_bullets = nullptr;
	m_curBulletCount = 0;

	delete m_playerShip;
	m_playerShip = nullptr; 

	delete m_uiCamera;
	m_uiCamera = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;
}


//-----------------------------------------------------------------------------------------------
void Game::RestartGame()
{
	Shutdown();
	Startup();
	m_curGameState = GameState::GAME_STATE_ATTRACT;
}


//-----------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	m_playerShip->Update(deltaSeconds);

	if ( m_curGameRestartTimer > 0.f )
	{
		m_curGameRestartTimer -= deltaSeconds;
		if ( m_curGameRestartTimer <= 0.f )
		{
			RestartGame();
			return;
		}

	}
	if ( m_curGameState == GameState::GAME_STATE_ATTRACT )
	{
		// Cycle through different colors for the logo in a slow, non-epilepsy-inducing way
		float colorChangeRate = deltaSeconds * 80;

		m_logoColor.r += (unsigned char)( colorChangeRate * m_logoRedChangeFactor );
		m_logoColor.g += (unsigned char)( colorChangeRate * m_logoGreenChangeFactor );
		m_logoColor.b += (unsigned char)( colorChangeRate * m_logoBlueChangeFactor );

		if ( m_logoColor.r < 30
			 || m_logoColor.r > 250)
		{
			m_logoRedChangeFactor *= -1;
			m_logoColor.r += (unsigned char)( 2.f * m_logoRedChangeFactor );
		}
		if ( m_logoColor.g < 10
			 || m_logoColor.g > 250 )
		{
			m_logoGreenChangeFactor *= -1;
			m_logoColor.g += (unsigned char)( 5.f * m_logoGreenChangeFactor );
		}
		if ( m_logoColor.b < 50
			 || m_logoColor.b > 250 )
		{
			m_logoBlueChangeFactor *= -1;
			m_logoColor.b += (unsigned char)( 7.f * m_logoBlueChangeFactor );
		}

		UpdateCameras(deltaSeconds);
	}
	else if ( m_curGameState == GameState::GAME_STATE_PLAY )
	{
		UpdateEntityArray( m_bullets,		MAX_BULLETS,		deltaSeconds );
		UpdateEntityArray( m_asteroids,	MAX_ASTEROIDS,		deltaSeconds );
		UpdateEntityArray( m_beetles,		MAX_BEETLES,		deltaSeconds );
		UpdateEntityArray( m_wasps,		MAX_WASPS,			deltaSeconds );
		UpdateEntityArray( m_debrisPieces, MAX_DEBRIS_PIECES,	deltaSeconds );

		CheckCollisions();
		UpdateCameras(deltaSeconds);

		DeleteGarbageEntities();

		CheckForWaveChange();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear all screen (backbuffer) pixels to black
	// ALWAYS clear the screen at the top of each frame's Render()!
	g_renderer->ClearScreen(Rgba8(0, 0, 0));

	g_renderer->BeginCamera(*m_worldCamera);

	if ( m_curGameState == GameState::GAME_STATE_ATTRACT )
	{
		RenderAttractScreen();
	}
	else if ( m_curGameState == GameState::GAME_STATE_PLAY )
	{
		RenderEntityArray( m_bullets,		MAX_BULLETS );
		RenderEntityArray( m_asteroids,	MAX_ASTEROIDS );
		RenderEntityArray( m_beetles,		MAX_BEETLES );
		RenderEntityArray( m_wasps,		MAX_WASPS );
		RenderEntityArray( m_debrisPieces, MAX_DEBRIS_PIECES );

		// Render player ship last so the ship will appear on top of other objects when colliding
		m_playerShip->Render();
	}
	
	g_renderer->EndCamera(*m_worldCamera);

	// Render UI with a new camera
	g_renderer->BeginCamera( *m_uiCamera );
	
	if ( m_curGameState == GameState::GAME_STATE_PLAY )
	{
		m_playerShip->RenderRemainingLives();
		m_playerShip->RenderCooldowns();
	}

	g_renderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::CheckForWaveChange()
{
	// Check if game is shutting down
	if ( m_curGameRestartTimer > 0.f )
	{
		return;
	}

	if ( m_curAsteroidCount == 0
		&& m_curBeetleCount == 0
		&& m_curWaspCount == 0 )
	{
		m_curWave++;

		if ( m_curWave > NUM_WAVES )
		{
			SetGameState( GameState::GAME_STATE_ATTRACT );
			return;
		}

		StartWave(m_curWave);
	}
}


//-----------------------------------------------------------------------------------------------
void Game::RenderAttractScreen() const
{
	float pixelsPerUnit = 6.f;
	Vec2 offset( 4.5f, 5.f );
	float logoHeight = 6.f;

	// Build an array of each line position relative to bottom left of window
	Vec2 linePositions[] = 
	{
		// S
		Vec2( 0.f, 4.f ),
		Vec2( 2.f, logoHeight ),

		Vec2( 0.f, 4.f ),
		Vec2( 2.f, 2.f ),

		Vec2( 0.f, 0.f ),
		Vec2( 2.f, 2.f ),

		// T
		Vec2( 3.f, logoHeight - 1.f ),
		Vec2( 6.f, logoHeight ),

		Vec2( 4.5f, logoHeight - .6f ),
		Vec2( 4.5f, 0.f ),

		// A
		Vec2( 6.f, 0.f ),
		Vec2( 7.5f, logoHeight ),

		Vec2( 7.5f, logoHeight ),
		Vec2( 9.f, 0.f ),
		
		Vec2( 6.75f, 3.f ),
		Vec2( 8.1f, 3.5f ),

		// R 
		Vec2( 10.f, 0.f ),
		Vec2( 10.f, logoHeight ),

		Vec2( 10.f, logoHeight ),
		Vec2( 12.f, logoHeight - 1.f ),

		Vec2( 10.f, 3.5f ),
		Vec2( 12.f, logoHeight - 1.f ),
		
		Vec2( 10.f, 3.5f ),
		Vec2( 12.f, 0.f ),

		// S
		Vec2( 13.f, 4.f ),
		Vec2( 15.f, logoHeight ),

		Vec2( 13.f, 4.f ),
		Vec2( 15.f, 2.f ),

		Vec2( 13.f, 0.f ),
		Vec2( 15.f, 2.f ),

		// H
		Vec2( 16.f, 0.f ),
		Vec2( 16.f, logoHeight ),

		Vec2( 18.f, 0.f ),
		Vec2( 18.f, logoHeight ),

		Vec2( 16.f, 2.5f ),
		Vec2( 18.f, 3.5f ),

		// I
		Vec2( 20.f, 0.f ),
		Vec2( 20.f, logoHeight ),

		// P
		Vec2( 22.f, 0.f ),
		Vec2( 22.f, logoHeight ),

		Vec2( 22.f, logoHeight ),
		Vec2( 24.f, logoHeight - 1.f ),

		Vec2( 22.f, 3.5f ),
		Vec2( 24.f, logoHeight - 1.f )
	};

	// Iterate through the array 2 elements at a time to draw each line with the start and end positions
	constexpr int positionVecLength = sizeof( linePositions ) / sizeof( linePositions[0] );
	for ( int posIndex = 0; posIndex < positionVecLength; )
	{
		DrawLine( ( linePositions[posIndex] + offset ) * pixelsPerUnit,
				  ( linePositions[posIndex + 1] + offset ) * pixelsPerUnit,
				  m_logoColor,
				  STARSHIP_TITLE_LINE_WIDTH );

		// Advance to next pair of points
		posIndex += 2;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::CheckCollisions()
{
	bool playerHasTakenDamage = false;
	if ( !m_playerShip->IsDead() )
	{
		// Check player collision with asteroids
		for ( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; ++asteroidIndex )
		{
			Entity*& asteroid = m_asteroids[asteroidIndex];
			if ( asteroid == nullptr )
			{
				continue;
			}

			if ( DoDiscsOverlap(m_playerShip->GetPosition(), m_playerShip->GetPhysicsRadius(), asteroid->GetPosition(), asteroid->GetPhysicsRadius()) )
			{
				asteroid->TakeDamage(1);
				m_playerShip->TakeDamage(1);
				playerHasTakenDamage = true;
				break;
			}
		}
		if ( !playerHasTakenDamage )
		{
			// Check player collision with beetles
			for ( int beetleIndex = 0; beetleIndex < MAX_BEETLES; ++beetleIndex )
			{
				Entity*& beetle = m_beetles[beetleIndex];
				if ( beetle == nullptr )
				{
					continue;
				}

				if ( DoDiscsOverlap(m_playerShip->GetPosition(), m_playerShip->GetPhysicsRadius(), beetle->GetPosition(), beetle->GetPhysicsRadius()) )
				{
					beetle->TakeDamage(1);
					m_playerShip->TakeDamage(1);
					playerHasTakenDamage = true;
					break;
				}
			}
		}
		if ( !playerHasTakenDamage )
		{
			// Check player collision with wasps
			for ( int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex )
			{
				Entity*& wasp = m_wasps[waspIndex];
				if ( wasp == nullptr )
				{
					continue;
				}

				if ( DoDiscsOverlap(m_playerShip->GetPosition(), m_playerShip->GetPhysicsRadius(), wasp->GetPosition(), wasp->GetPhysicsRadius()) )
				{
					wasp->TakeDamage(1);
					m_playerShip->TakeDamage(1);
					playerHasTakenDamage = true;
					break;
				}
			}
		}
	}

	// Check bullet collisions
	for ( int bulletIndex = 0; bulletIndex < MAX_BULLETS; ++bulletIndex )
	{
		Entity*& bullet = m_bullets[bulletIndex];
		if ( bullet == nullptr )
		{
			continue;
		}

		// Check bullet collisions with asteroids
		for ( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; ++asteroidIndex )
		{
			Entity*& asteroid = m_asteroids[asteroidIndex];
			if ( asteroid == nullptr )
			{
				continue;
			}

			if ( DoDiscsOverlap( bullet->GetPosition(), bullet->GetPhysicsRadius(), asteroid->GetPosition(), asteroid->GetPhysicsRadius() ) )
			{
				asteroid->TakeDamage( 1 );
				bullet->TakeDamage( 1 );
				break;
			}
		}
		if ( bullet->IsGarbage() )
		{
			continue;
		}

		// Check bullet collisions with beetles
		for ( int beetleIndex = 0; beetleIndex < MAX_BEETLES; ++beetleIndex )
		{
			Entity*& beetle = m_beetles[beetleIndex];
			if ( beetle == nullptr )
			{
				continue;
			}

			if ( DoDiscsOverlap( bullet->GetPosition(), bullet->GetPhysicsRadius(), beetle->GetPosition(), beetle->GetPhysicsRadius() ) )
			{
				beetle->TakeDamage( 1 );
				bullet->TakeDamage( 1 );
				break;
			}
		}
		if ( bullet->IsGarbage() )
		{
			continue;
		}

		// Check bullet collisions with wasps
		for ( int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex )
		{
			Entity*& wasp = m_wasps[waspIndex];
			if ( wasp == nullptr )
			{
				continue;
			}

			if ( DoDiscsOverlap( bullet->GetPosition(), bullet->GetPhysicsRadius(), wasp->GetPosition(), wasp->GetPhysicsRadius() ) )
			{
				wasp->TakeDamage( 1 );
				bullet->TakeDamage( 1 );
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::AcceleratePlayerShip()
{
	m_playerShip->AccelerateShip();
}


//-----------------------------------------------------------------------------------------------
void Game::TurnPlayerShipLeft()
{
	m_playerShip->TurnShipLeft();
}


//-----------------------------------------------------------------------------------------------
void Game::TurnPlayerShipRight()
{
	m_playerShip->TurnShipRight();
}


//-----------------------------------------------------------------------------------------------
void Game::StopAcceleratingPlayerShip()
{
	m_playerShip->StopAcceleratingShip();
} 


//-----------------------------------------------------------------------------------------------
void Game::StopTurningPlayerShipLeft()
{
	m_playerShip->StopTurningShipLeft();
}


//-----------------------------------------------------------------------------------------------
void Game::StopTurningPlayerShipRight()
{
	m_playerShip->StopTurningShipRight();
}


//-----------------------------------------------------------------------------------------------
void Game::HandleBulletFired()
{
	if ( m_curGameState == GameState::GAME_STATE_ATTRACT )
	{
		SetGameState(GameState::GAME_STATE_PLAY);
		return;
	}

	m_playerShip->HandleBulletFired();	
}


//-----------------------------------------------------------------------------------------------
void Game::HandleStarburstBulletFired()
{
	m_playerShip->HandleStarburstBulletFired();
}


//-----------------------------------------------------------------------------------------------
void Game::HandleBulletReload()
{
	if ( !m_playerShip->IsDead() )
	{
		m_playerShip->HandleBulletReload();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnBullet( const Vec2& pos, const Vec2& orientation )
{
	if ( m_curBulletCount < MAX_BULLETS )
	{
		for ( int bulletIndex = 0; bulletIndex < MAX_BULLETS; ++bulletIndex )
		{
			Entity*& bullet = m_bullets[bulletIndex];
			if ( bullet == nullptr )
			{
				bullet = new Bullet( this, pos, orientation );
				m_curBulletCount++;
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnStarburstBullet(const Vec2& pos, const Vec2& orientation)
{
	if ( m_curBulletCount < MAX_BULLETS )
	{
		for ( int bulletIndex = 0; bulletIndex < MAX_BULLETS; ++bulletIndex )
		{
			Entity*& bullet = m_bullets[bulletIndex];
			if ( bullet == nullptr )
			{
				bullet = new StarburstBullet(this, pos, orientation);
				m_curBulletCount++;
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::RespawnPlayerIfDead()
{
	if ( m_curGameState == GameState::GAME_STATE_ATTRACT )
	{
		SetGameState( GameState::GAME_STATE_PLAY );
		return;
	}

	if ( m_playerShip->IsDead() )
	{
		m_playerShip->Respawn();
	}
}


//-----------------------------------------------------------------------------------------------
Vec2 Game::GetPlayerPosition() const
{
	// Return impossible position to specify the player is dead
	// TODO: Replace this with a non-magic value
	if ( m_playerShip == nullptr 
		|| m_playerShip->IsDead() )
	{
		return Vec2( -1.f, -1.f );
	}
	else
	{
		return m_playerShip->GetPosition();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnMultipleAsteroids( int numAsteroids )
{
	for ( int asteroidIndex = 0; asteroidIndex < numAsteroids; ++asteroidIndex )
	{
		SpawnAsteroid();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnAsteroid()
{
	if ( m_curAsteroidCount < MAX_ASTEROIDS )
	{
		for ( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; ++asteroidIndex )
		{
			Entity*& asteroid = m_asteroids[asteroidIndex];
			if ( asteroid == nullptr )
			{
				// Make sure asteroid doesn't spawn too close to the player ship
				Vec2 position;
				do 
				{
					position = Vec2( m_randNumGen->RollRandomFloatInRange( 0, WINDOW_WIDTH ), m_randNumGen->RollRandomFloatInRange( 0, WINDOW_HEIGHT ) );
				} while ( GetDistance2D( position, m_playerShip->GetPosition() ) < MIN_ASTEROID_SPAWN_DIST_FROM_PLAYER );

				asteroid = new Asteroid( this, position );
				m_curAsteroidCount++;
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnBeetles( int numBeetles )
{
	for ( int beetleCount = 0; beetleCount < numBeetles; ++beetleCount )
	{
		if ( m_curBeetleCount < MAX_BEETLES )
		{
			for ( int beetleIndex = 0; beetleIndex < MAX_BEETLES; ++beetleIndex )
			{
				Entity*& beetles = m_beetles[beetleIndex];
				if ( beetles == nullptr )
				{
					Vec2 position;
					int spawnQuadrant = m_randNumGen->RollRandomIntLessThan(4);

					switch ( spawnQuadrant )
					{
						// Top
						case 0:
							position = Vec2(m_randNumGen->RollRandomFloatInRange(-10.f, WINDOW_WIDTH + 10.f),
											m_randNumGen->RollRandomFloatInRange(WINDOW_HEIGHT + 10.f, WINDOW_HEIGHT + 50.f));
							break;
							// Left
						case 1:
							position = Vec2(m_randNumGen->RollRandomFloatInRange(-50.f, -10.f),
											m_randNumGen->RollRandomFloatInRange(-10.f, WINDOW_HEIGHT + 10.f));
							break;
							// Right
						case 2:
							position = Vec2(m_randNumGen->RollRandomFloatInRange(WINDOW_WIDTH + 10.f, WINDOW_WIDTH + 50.f),
											m_randNumGen->RollRandomFloatInRange(-10.f, WINDOW_HEIGHT + 10.f));
							break;
							// Bottom
						case 3:
							position = Vec2(m_randNumGen->RollRandomFloatInRange(-10.f, WINDOW_WIDTH + 10.f),
											m_randNumGen->RollRandomFloatInRange(-50.f, -10.f));
							break;
					}

					beetles = new Beetle(this, position);
					m_curBeetleCount++;
					break;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnWasps( int numWasps )
{
	for ( int waspCount = 0; waspCount < numWasps; ++waspCount )
	{
		if ( m_curWaspCount < MAX_WASPS )
		{
			for ( int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex )
			{
				Entity*& wasp = m_wasps[waspIndex];
				if ( wasp == nullptr )
				{
					Vec2 position;
					int spawnQuadrant = m_randNumGen->RollRandomIntLessThan(4);
					
					switch ( spawnQuadrant )
					{
						// Top
						case 0:
							position = Vec2(m_randNumGen->RollRandomFloatInRange(-10.f, WINDOW_WIDTH + 10.f),
											m_randNumGen->RollRandomFloatInRange(WINDOW_HEIGHT + 10.f, WINDOW_HEIGHT + 50.f));
						break;
						// Left
						case 1:
							position = Vec2(m_randNumGen->RollRandomFloatInRange(-50.f, -10.f),
											m_randNumGen->RollRandomFloatInRange(-10.f, WINDOW_HEIGHT + 10.f));
							break;
						// Right
						case 2:
							position = Vec2(m_randNumGen->RollRandomFloatInRange(WINDOW_WIDTH + 10.f, WINDOW_WIDTH + 50.f),
											m_randNumGen->RollRandomFloatInRange(-10.f, WINDOW_HEIGHT + 10.f));
							break;
						// Bottom
						case 3:
							position = Vec2(m_randNumGen->RollRandomFloatInRange(-10.f, WINDOW_WIDTH + 10.f),
											m_randNumGen->RollRandomFloatInRange(-50.f, -10.f));
							break;
					}

					wasp = new Wasp(this, position);
					m_curWaspCount++;
					break;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	// World camera
	m_screenShakeIntensity -= SCREEN_SHAKE_ABLATION_PER_SECOND * deltaSeconds;
	m_screenShakeIntensity = ClampMinMax(m_screenShakeIntensity, 0.f, 1.0);

	float maxScreenShake = m_screenShakeIntensity * MAX_CAMERA_SHAKE_DIST;
	float cameraShakeX = m_randNumGen->RollRandomFloatInRange(-maxScreenShake, maxScreenShake);
	float cameraShakeY = m_randNumGen->RollRandomFloatInRange(-maxScreenShake, maxScreenShake);
	Vec2 cameraShakeOffset = Vec2(cameraShakeX, cameraShakeY);

	m_worldCamera->SetOrthoView(Vec2(0.f, 0.f), Vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
	m_worldCamera->Translate2D(cameraShakeOffset);

	// UI Camera
	m_uiCamera->SetOrthoView( Vec2( 0.f, 0.f ), Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );

}


//-----------------------------------------------------------------------------------------------
void Game::UpdateEntityArray( Entity** entities, int entityCount, float deltaSeconds )
{
	for ( int entityIndex = 0; entityIndex < entityCount; ++entityIndex )
	{
		Entity*& entity = entities[entityIndex];
		if ( entity != nullptr )
		{
			entity->Update( deltaSeconds );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::RenderEntityArray( Entity** entities, int entityCount ) const
{
	for ( int entityIndex = 0; entityIndex < entityCount; ++entityIndex )
	{
		Entity*& entity = entities[entityIndex];
		if ( entity != nullptr )
		{
			entity->Render();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRenderEntityArray( Entity** entities, int entityCount ) const
{
	for ( int entityIndex = 0; entityIndex < entityCount; ++entityIndex )
	{
		Entity*& entity = entities[entityIndex];
		if ( entity != nullptr )
		{
			entity->DebugRender();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	if ( m_curGameState == GameState::GAME_STATE_PLAY )
	{
		m_playerShip->DebugRender();
		DrawDebugLinesToPlayer();

		DebugRenderEntityArray( m_bullets,		MAX_BULLETS );
		DebugRenderEntityArray( m_asteroids,	MAX_ASTEROIDS );
		DebugRenderEntityArray( m_beetles,		MAX_BEETLES );
		DebugRenderEntityArray( m_wasps,		MAX_WASPS );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::DrawDebugLinesToPlayer() const
{
	for ( int bulletIndex = 0; bulletIndex < MAX_BULLETS; ++bulletIndex )
	{
		Entity*& bullet = m_bullets[bulletIndex];

		if ( bullet != nullptr )
		{
			DrawLine( bullet->GetPosition(), m_playerShip->GetPosition(), Rgba8( 50, 50, 50 ), DEBUG_LINE_THICKNESS );
		}
	}

	for ( int asteroidIndex = 0; asteroidIndex < MAX_ASTEROIDS; ++asteroidIndex )
	{
		Entity*& asteroid = m_asteroids[asteroidIndex];

		if ( asteroid != nullptr )
		{
			DrawLine( asteroid->GetPosition(), m_playerShip->GetPosition(), Rgba8( 50, 50, 50 ), DEBUG_LINE_THICKNESS );
		}
	}

	for ( int beetleIndex = 0; beetleIndex < MAX_BEETLES; ++beetleIndex )
	{
		Entity*& beetle = m_beetles[beetleIndex];

		if ( beetle != nullptr )
		{
			DrawLine( beetle->GetPosition(), m_playerShip->GetPosition(), Rgba8( 50, 50, 50 ), DEBUG_LINE_THICKNESS );
		}
	}

	for ( int waspIndex = 0; waspIndex < MAX_WASPS; ++waspIndex )
	{
		Entity*& wasp = m_wasps[waspIndex];

		if ( wasp != nullptr )
		{
			DrawLine( wasp->GetPosition(), m_playerShip->GetPosition(), Rgba8( 50, 50, 50 ), DEBUG_LINE_THICKNESS );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::DeleteGarbageEntities()
{
	DeleteGarbageEntitiesInArray( m_bullets,		MAX_BULLETS,		m_curBulletCount );
	DeleteGarbageEntitiesInArray( m_asteroids,		MAX_ASTEROIDS,		m_curAsteroidCount );
	DeleteGarbageEntitiesInArray( m_beetles,		MAX_BEETLES,		m_curBeetleCount );
	DeleteGarbageEntitiesInArray( m_wasps,			MAX_WASPS,			m_curWaspCount );
	DeleteGarbageEntitiesInArray( m_debrisPieces,	MAX_DEBRIS_PIECES,	m_curDebrisPiecesCount );
}


//-----------------------------------------------------------------------------------------------
void Game::DeleteGarbageEntitiesInArray( Entity** entities, int entityCount, int& curEntityCount )
{
	for ( int entityIndex = 0; entityIndex < entityCount; ++entityIndex )
	{
		Entity*& entity = entities[entityIndex];

		if ( entity != nullptr
			 && entity->IsGarbage() )
		{
			delete entity;
			entity = nullptr;

			--curEntityCount;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::StartWave(int waveNum)
{
	switch ( waveNum )
	{
		case 1:
			SpawnMultipleAsteroids(8);
			SpawnBeetles(2);
			break;
		case 2:
			SpawnMultipleAsteroids(12);
			SpawnBeetles(4);
			SpawnWasps(1);
			break;
		case 3:
			SpawnMultipleAsteroids(15);
			SpawnBeetles(8);
			SpawnWasps(3);
			break;
		case 4:
			SpawnMultipleAsteroids(7);
			SpawnBeetles(10);
			SpawnWasps(7);
			break;
		case 5:
			SpawnMultipleAsteroids(8);
			SpawnBeetles(8);
			SpawnWasps(12);
			break;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::AddScreenShakeIntensity(float intensity)
{
	m_screenShakeIntensity += intensity;
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnDebris( const Vec2& position, const Vec2& velocity, const Rgba8& color, int numPieces )
{
	if ( m_curDebrisPiecesCount < MAX_DEBRIS_PIECES )
	{
		for ( int i = 0; i < numPieces; i++ )
		{
			for ( int debrisIndex = 0; debrisIndex < MAX_DEBRIS_PIECES; ++debrisIndex )
			{
				Entity*& debrisPiece = m_debrisPieces[debrisIndex];
				if ( debrisPiece == nullptr )
				{
					float randomOrientationOffsetDegrees = m_randNumGen->RollRandomFloatInRange( -30.f, 30.f );
					Vec2 newVelocity = Vec2::MakeFromPolarDegrees( velocity.GetOrientationDegrees() + randomOrientationOffsetDegrees, 
																   velocity.GetLength() * m_randNumGen->RollRandomFloatInRange( .25f, 3.f ) );
					debrisPiece = new Debris( this, position, newVelocity, color );
					m_curDebrisPiecesCount++;
					break;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SetGameState(GameState gameState)
{
	if ( gameState == GameState::GAME_STATE_ATTRACT )
	{
		m_curGameRestartTimer = GAME_RESET_DELAY_SECONDS;
	}
	else if ( gameState == GameState::GAME_STATE_PLAY )
	{
		StartWave(m_curWave);
		m_curGameState = gameState;
	}
}

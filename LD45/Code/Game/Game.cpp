#include "Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Game/GameCommon.hpp"
#include "Game/World.hpp"
#include "Game/Entity.hpp"
#include "Game/Player.hpp"
#include "Game/Enemy.hpp"
#include "Game/EnemySquare.hpp"


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
	m_world = new World( this );

	m_worldCamera = new Camera();
	m_uiCamera = new Camera();

	m_randNumGen = new RandomNumberGenerator();

	m_player = new Player( this, Vec2( 0.f, 0.f ) );
	m_entities.push_back( m_player );
	
	MakePrettyTriangles();

	for ( int i = 0; i < 20; i++ )
	{
		m_entities.push_back( new Enemy( this, Vec2( m_randNumGen->GetRandomFloatInRange( 20.f, WORLD_SIZE_X - 20.f ),
													 m_randNumGen->GetRandomFloatInRange( 20.f, WORLD_SIZE_Y - 20.f ) ),
											   Rgba8( m_randNumGen->GetRandomIntInRange( 30, 250 ),
											   		  m_randNumGen->GetRandomIntInRange( 30, 250 ),
													  m_randNumGen->GetRandomIntInRange( 30, 250 ) ) ) );
	}

	for ( int i = 0; i < 5; i++ )
	{
		m_entities.push_back( new EnemySquare( this, 1.f, Vec2( m_randNumGen->GetRandomFloatInRange( 200.f, WORLD_SIZE_X - 20.f ),
														   m_randNumGen->GetRandomFloatInRange( 200.f, WORLD_SIZE_Y - 20.f ) ),
													Rgba8( m_randNumGen->GetRandomIntInRange( 30, 250 ),
											   			   m_randNumGen->GetRandomIntInRange( 30, 250 ),
														   m_randNumGen->GetRandomIntInRange( 30, 250 ) ) ) );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	while ( (int)m_entities.size() > 0)
	{
		m_entities.pop_back();
	}

	delete m_randNumGen;
	m_randNumGen = nullptr;
	
	delete m_uiCamera;
	m_uiCamera = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_world;
	m_world = nullptr;
}


//-----------------------------------------------------------------------------------------------
void Game::RestartGame()
{
	Shutdown();
	Startup();
}


//-----------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	SpawnNewEntities( deltaSeconds );

	UpdateEntities( deltaSeconds );

	CheckCollisions();
	UpdateCameras( deltaSeconds );

	DeleteGarbageEntities();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear all screen (backbuffer) pixels to black
	// ALWAYS clear the screen at the top of each frame's Render()!
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0));

	g_theRenderer->BeginCamera(*m_worldCamera);

	m_world->Render();
	RenderEntities();
	if ( m_isDebugRendering )
	{
		DebugRenderEntities();
	}

	g_theRenderer->EndCamera(*m_worldCamera);

	// Render UI with a new camera
	g_theRenderer->BeginCamera( *m_uiCamera );
	
	g_theRenderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::CheckCollisions()
{
	// Check player collision against everything else (start at 1 so player doesn't check collision with itself)
	for ( int entityIndex = 1; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		if( entity == nullptr
			|| entity->GetEntityType() == EntityType::ENTITY_TYPE_PLAYER_TRIANGLE )
		{
			continue;
		}

		if ( entity->GetEntityType() == EntityType::ENTITY_TYPE_TRIANGLE )
		{
			if ( DoDiscsOverlap( m_player->GetPosition(), m_player->GetPhysicsRadius(), entity->GetPosition(), entity->GetPhysicsRadius() ) )
			{
				m_player->CaptureEntity( entity );
				entity->BecomeCaptured();
				break;
			}
		}
		else if ( !m_player->IsInvincible()
				  && entity->GetEntityType() == EntityType::ENTITY_TYPE_BOX )
		{
			if ( DoDiscAndAABBOverlap2D( m_player->GetPosition(), m_player->GetPhysicsRadius(), entity->GetPhysicsBox()) )
			{
				m_player->TakeDamage();
				m_player->AddImpulse( 50.f, GetNormalizedDirectionFromAToB( entity->GetPosition(), m_player->GetPosition() ) );
				break;
			}
		}
	}

	// Check for triangle box collisions
	for ( int entity1Index = 1; entity1Index < (int)m_entities.size(); ++entity1Index )
	{
		for ( int entity2Index = 1; entity2Index < (int)m_entities.size(); ++entity2Index )
		{
			Entity*& entity1 = m_entities[entity1Index];
			Entity*& entity2 = m_entities[entity2Index];
			if ( entity1Index == entity2Index
				 || entity1 == nullptr
				 || entity2 == nullptr )
			{
				continue;
			}

			if ( entity1->GetEntityType() == EntityType::ENTITY_TYPE_PLAYER_TRIANGLE
				 && entity2->GetEntityType() == EntityType::ENTITY_TYPE_BOX )
			{
				// Player triangle is colliding
				if ( DoDiscAndAABBOverlap2D( entity1->GetPosition(), entity1->GetPhysicsRadius(), entity2->GetPhysicsBox() ) )
				{
					entity2->AttachAttacker( entity1 );
					entity1->TakeDamage( 1 );
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
	float cameraShakeX = m_randNumGen->GetRandomFloatInRange(-maxScreenShake, maxScreenShake);
	float cameraShakeY = m_randNumGen->GetRandomFloatInRange(-maxScreenShake, maxScreenShake);
	Vec2 cameraShakeOffset = Vec2(cameraShakeX, cameraShakeY);

	Vec2 bottomLeft( m_player->GetPosition().x - WINDOW_WIDTH / 2.f, m_player->GetPosition().y - WINDOW_HEIGHT / 2.f );
	Vec2 topRight( m_player->GetPosition().x + WINDOW_WIDTH / 2.f, m_player->GetPosition().y + WINDOW_HEIGHT / 2.f );

	if ( bottomLeft.x < 0 ) 
	{ 
		bottomLeft.x = 0; 
		topRight.x = bottomLeft.x + WINDOW_WIDTH;
	}
	if ( bottomLeft.y < 0 ) 
	{ 
		bottomLeft.y = 0;
		topRight.y = bottomLeft.y + WINDOW_HEIGHT;
	}

	if ( topRight.x >  WORLD_SIZE_X ) 
	{ 
		topRight.x = WORLD_SIZE_X;
		bottomLeft.x = topRight.x - WINDOW_WIDTH;
	}
	if ( topRight.y >  WORLD_SIZE_Y ) 
	{ 
		topRight.y = WORLD_SIZE_Y;
		bottomLeft.y = topRight.y - WINDOW_HEIGHT;
	}

	m_worldCamera->SetOrthoView( bottomLeft, topRight );
	m_worldCamera->Translate2D(cameraShakeOffset);

	// UI Camera
	m_uiCamera->SetOrthoView( Vec2( 0.f, 0.f ), Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateEntities( float deltaSeconds )
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];
		if ( entity != nullptr
			 && !entity->IsCaptured() )
		{
			entity->Update( deltaSeconds );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::RenderEntities() const
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		if ( m_entities[entityIndex] != nullptr ) 
			 //&& !m_entities[entityIndex]->IsCaptured())
		{
			m_entities[entityIndex]->Render();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRenderEntities() const
{
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		if ( m_entities[entityIndex] != nullptr )
		{
			m_entities[entityIndex]->DebugRender();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
}


//-----------------------------------------------------------------------------------------------
void Game::DeleteGarbageEntities()
{
	//std::vector<Entity*> deadEntities;
	for ( int entityIndex = 0; entityIndex < (int)m_entities.size(); ++entityIndex )
	{
		Entity*& entity = m_entities[entityIndex];

		if ( entity != nullptr
			 && entity->IsGarbage() )
		{
			//deadEntities.push_back( entity );
			//delete entity;
			entity = nullptr;
		}
	}

	/*for ( int entityIndex = 0; entityIndex < (int)deadEntities.size(); ++entityIndex )
	{
		Entity*& deadEntity = deadEntities[entityIndex];
		delete deadEntities[entityIndex];
		deadEntity = nullptr;
	}*/
}

void Game::SpawnNewEntities( float deltaSeconds )
{
	if ( m_triangleSpawnTimer <= 0.f )
	{
		m_entities.push_back( new Enemy( this, Vec2( m_randNumGen->GetRandomFloatInRange( 20.f, WORLD_SIZE_X - 20.f ),
													 m_randNumGen->GetRandomFloatInRange( 20.f, WORLD_SIZE_Y - 20.f ) ),
											  Rgba8( m_randNumGen->GetRandomIntInRange( 30, 250 ),
											   		 m_randNumGen->GetRandomIntInRange( 30, 250 ),
													 m_randNumGen->GetRandomIntInRange( 30, 250 ) ) ) );
		m_triangleSpawnTimer = TRIANGLE_SPAWN_TIMER;
	}

	if ( m_boxSpawnTimer <= 0.f )
	{
		for ( int boxIndex = 0; boxIndex < BOX_SPAWN_COUNT; ++boxIndex )
		{
			m_entities.push_back( new EnemySquare( this, m_boxSizeModifier,
												   Vec2( m_randNumGen->GetRandomFloatInRange( 20.f, WORLD_SIZE_X - 20.f ),
														 m_randNumGen->GetRandomFloatInRange( 20.f, WORLD_SIZE_Y - 20.f ) ),
												   Rgba8( m_randNumGen->GetRandomIntInRange( 30, 250 ),
														  m_randNumGen->GetRandomIntInRange( 30, 250 ),
														  m_randNumGen->GetRandomIntInRange( 30, 250 ) ) ) );
		}

		m_boxSizeModifier += .25f;
		m_boxSpawnTimer = BOX_SPAWN_TIMER;
	}

	m_triangleSpawnTimer -= deltaSeconds;
	m_boxSpawnTimer -= deltaSeconds;
}

//-----------------------------------------------------------------------------------------------
void Game::MakePrettyTriangles()
{
	std::vector<Rgba8> colors;
	colors.push_back( Rgba8( 0, 0, 255 ) );
	colors.push_back( Rgba8( 0, 255, 0 ) );
	colors.push_back( Rgba8( 255, 0, 0 ) );
	colors.push_back( Rgba8( 255, 0, 255 ) );
	colors.push_back( Rgba8( 255, 255, 0 ) );
	colors.push_back( Rgba8( 0, 255, 255 ) );

	for ( int colorIndex = 0; colorIndex < (int)colors.size(); ++colorIndex )
	{
		{
			std::vector<Vertex_PCU> triangleVertexes;

			Vec2 vertex1( -1.f, 0.f );
			Vec2 vertex2( 2.f, 0.f );
			Vec2 vertex3( -1.f, 2.f );
			Rgba8 color = colors[colorIndex];

			triangleVertexes.push_back( Vertex_PCU( vertex1, color ) );
			triangleVertexes.push_back( Vertex_PCU( vertex2, color ) );
			triangleVertexes.push_back( Vertex_PCU( vertex3, color ) );

			m_triangleVertexGroups.push_back( triangleVertexes );
		}
	}
}


//-----------------------------------------------------------------------------------------------
std::vector<Vertex_PCU> Game::GetPrettyTriangle()
{
	std::vector<Vertex_PCU> triangle = m_triangleVertexGroups[m_randNumGen->GetRandomIntLessThan( (int)m_triangleVertexGroups.size() )];

	float newRotDegrees = m_randNumGen->GetRandomFloatInRange( -90.f, 180.f );
	triangle[0].m_position = triangle[0].m_position.GetRotatedAboutZDegrees( newRotDegrees );
	triangle[1].m_position = triangle[1].m_position.GetRotatedAboutZDegrees( newRotDegrees );
	triangle[2].m_position = triangle[2].m_position.GetRotatedAboutZDegrees( newRotDegrees );

	float newScale = m_randNumGen->GetRandomFloatInRange( 1.f, 3.f );
	triangle[0].m_position = triangle[0].m_position * newScale;
	triangle[1].m_position = triangle[1].m_position * newScale;
	triangle[2].m_position = triangle[2].m_position * newScale;

	return triangle;
}


//-----------------------------------------------------------------------------------------------
void Game::AddScreenShakeIntensity(float intensity)
{
	m_screenShakeIntensity += intensity;

	ClampMinMax( m_screenShakeIntensity, 0.f, 1.f );
}


//-----------------------------------------------------------------------------------------------
void Game::AddVertexesToPlayer( std::vector<Vertex_PCU> vertexes, const Rgba8& color )
{
	m_player->AddVertexes( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnEntity( Entity* entity, const Vec2& position, const Vec2& initialVelocity )
{
	entity->Reset();
	entity->SetPosition( position );
	entity->SetVelocity( initialVelocity );
	entity->SetCaptured( false );
	

	m_entities.push_back( entity );
}

//-----------------------------------------------------------------------------------------------
void Game::SpawnEnemy( std::vector<Vertex_PCU> vertexes, const Rgba8& color, const Vec2& position, const Vec2& initialVelocity )
{
	Enemy* newEnemy = new Enemy( this, position, vertexes, color );
	m_entities.push_back( newEnemy );

	SpawnEntity( newEnemy, position, initialVelocity );
}
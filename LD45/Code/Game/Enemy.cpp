#include "Game/Enemy.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Enemy::Enemy( Game* game, Vec2 position, const Rgba8& color )
	: Entity(game, position)
{
	m_entityType = EntityType::ENTITY_TYPE_TRIANGLE;
	//m_health = 2;

	m_vertexes = m_game->GetPrettyTriangle();

	m_mainColor = m_vertexes[0].m_color;

	float vertexLength0 = m_vertexes[0].m_position.GetLength();
	float vertexLength1 = m_vertexes[1].m_position.GetLength();
	float vertexLength2 = m_vertexes[2].m_position.GetLength();

	if ( vertexLength0 > vertexLength1
		 && vertexLength0 > vertexLength2 )
	{
		m_physicsRadius = vertexLength0;
	}
	else if ( vertexLength1 > vertexLength0
				&& vertexLength1 > vertexLength2 )
	{
		m_physicsRadius = vertexLength1;
	}
	else
	{
		m_physicsRadius = vertexLength2;
	}
}


//-----------------------------------------------------------------------------------------------
Enemy::Enemy( Game* game, Vec2 position, std::vector<Vertex_PCU> vertexes, const Rgba8& color )
	: Entity( game, position )
{
	m_entityType = EntityType::ENTITY_TYPE_TRIANGLE;

	m_physicsRadius = 2.f;
	m_mainColor = color;
	m_vertexes = vertexes;

	float vertexLength0 = m_vertexes[0].m_position.GetLength();
	float vertexLength1 = m_vertexes[1].m_position.GetLength();
	float vertexLength2 = m_vertexes[2].m_position.GetLength();

	if ( vertexLength0 > vertexLength1
		 && vertexLength0 > vertexLength2 )
	{
		m_physicsRadius = vertexLength0;
	}
	else if ( vertexLength1 > vertexLength0
			  && vertexLength1 > vertexLength2 )
	{
		m_physicsRadius = vertexLength1;
	}
	else
	{
		m_physicsRadius = vertexLength2;
	}
}

//-----------------------------------------------------------------------------------------------
Enemy::~Enemy()
{
}


//-----------------------------------------------------------------------------------------------
void Enemy::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );

	CheckIfOutOfBounds();
	CheckIfStopped();
	
	m_vertexesCopy = m_vertexes;
	Vertex_PCU::TransformVertexArray( m_vertexesCopy, 1.f, m_orientationDegrees, m_position );
}


void Enemy::CheckIfOutOfBounds()
{
	if ( m_position.x + m_physicsRadius > WORLD_SIZE_X
		 || m_position.x - m_physicsRadius < 0.f
		 || m_position.y + m_physicsRadius > WORLD_SIZE_Y
		 || m_position.y -m_physicsRadius < 0.f )
	{
		Die();
	}
}


void Enemy::CheckIfStopped()
{
	if ( m_velocity.GetLength() < .1f )
	{
		m_entityType = EntityType::ENTITY_TYPE_TRIANGLE;
	}
}

//-----------------------------------------------------------------------------------------------
void Enemy::Render() const
{
	if ( IsDead() )
	{
		return;
	}
	
	g_theRenderer->DrawVertexArray( m_vertexesCopy );
}


//-----------------------------------------------------------------------------------------------
void Enemy::Die()
{
	m_isGarbage = true;
	//m_velocity = Vec2( 0.f, 0.f );

	Entity::Die();
}


//-----------------------------------------------------------------------------------------------
void Enemy::Collide( const Entity* otherEntity )
{
	//otherEntity
}


//-----------------------------------------------------------------------------------------------
void Enemy::DebugRender() const
{
	Entity::DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Enemy::BecomeCaptured()
{
	m_game->AddVertexesToPlayer( m_vertexes, m_mainColor );
	m_capturedByPlayer = true;

	m_game->AddScreenShakeIntensity( .15f );

	Die();
}

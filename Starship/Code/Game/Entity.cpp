#include "Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Entity::Entity( Game* theGame, const Vec2& position)
	: m_game( theGame )
	, m_position( position )
{
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	// vel += acceleration * dt;
	/*m_velocity += m_linearAcceleration * deltaSeconds;
	m_linearAcceleration = Vec2( 0.f, 0.f );*/
	// pos += vel * dt;
	m_position += m_velocity * deltaSeconds;

	//update orientation
	m_orientationDegrees += m_angularVelocity * deltaSeconds;

	if ( m_hitFlashTimer > 0.f )
	{
		m_hitFlashTimer -= deltaSeconds;
	}
	else
	{
		m_hitFlashTimer = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	m_isDead = true;
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	Vec2 end = m_position +  m_velocity;
	DrawLine( m_position, end, Rgba8(255, 255, 0), DEBUG_LINE_THICKNESS );
	DrawRing( m_position, m_physicsRadius, Rgba8(0, 255, 255), DEBUG_LINE_THICKNESS );
	DrawRing( m_position, m_cosmeticRadius, Rgba8(255, 0, 255), DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsOffScreen() const
{
	if ( m_position.x - m_cosmeticRadius >= WINDOW_WIDTH + MAX_CAMERA_SHAKE_DIST
		 || m_position.x + m_cosmeticRadius <= -MAX_CAMERA_SHAKE_DIST)
	{
		return true;
	}

	if ( m_position.y - m_cosmeticRadius >= WINDOW_HEIGHT + MAX_CAMERA_SHAKE_DIST
		 || m_position.y + m_cosmeticRadius <= -MAX_CAMERA_SHAKE_DIST)
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Entity::TakeDamage( int damage )
{
	m_health -= damage;
	if ( m_health <= 0 )
	{
		m_game->SpawnDebris(m_position, m_velocity, m_mainColor, m_numDebrisPieces);
		Die();
	}
	
	m_hitFlashTimer = HIT_FLASH_SECONDS;

	m_game->AddScreenShakeIntensity(.05f);
}

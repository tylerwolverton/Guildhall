#include "Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
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
	m_velocity += m_linearAcceleration * deltaSeconds;
	m_linearAcceleration = Vec2( 0.f, 0.f );
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
void Entity::Collide( const Entity* otherEntity )
{

}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	float mappedX = RangeMapFloat( 0.f, WORLD_SIZE_X, 0.f, WINDOW_WIDTH, m_position.x );
	float mappedY = RangeMapFloat( 0.f, WORLD_SIZE_Y, 0.f, WINDOW_HEIGHT, m_position.y );
	Vec2 drawPos( mappedX, mappedY );

	Vec2 end = drawPos +  m_velocity;
	//DrawLine( drawPos, end, Rgba8(255, 255, 0), DEBUG_LINE_THICKNESS );
	//DrawRing( drawPos, m_physicsRadius, Rgba8(0, 255, 255), DEBUG_LINE_THICKNESS );
	DrawRing( m_position, m_physicsRadius, Rgba8(0, 255, 255), DEBUG_LINE_THICKNESS );
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
		Die();
	}
	
	m_hitFlashTimer = HIT_FLASH_SECONDS;

	m_game->AddScreenShakeIntensity(.05f);
}


//-----------------------------------------------------------------------------------------------
void Entity::BecomeCaptured()
{

}


//-----------------------------------------------------------------------------------------------
void Entity::AttachAttacker( Entity* attacker )
{

}


//-----------------------------------------------------------------------------------------------
void Entity::Reset()
{
	m_position = Vec2( 0.f, 0.f );
	m_velocity = Vec2( 0.f, 0.f );
	m_linearAcceleration = Vec2( 0.f, 0.f );
	m_orientationDegrees = 0.f;
	m_angularVelocity = 0.f;
	m_health = 1;
	m_isDead = false;
	m_isGarbage = false;
	m_numDebrisPieces = 0;
	m_hitFlashTimer = 0.f;

	m_isCaptured = false;
}

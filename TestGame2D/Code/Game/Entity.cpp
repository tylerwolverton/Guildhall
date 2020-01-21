#include "Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Entity::Entity( EntityType type, EntityFaction faction, const Vec2& position )
	: m_type( type )
	, m_faction( faction )
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

	ApplyFriction();

	UpdateHitFlash( deltaSeconds );
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
	g_renderer->BindTexture( nullptr );
	g_renderer->DrawRing2D( m_position, m_physicsRadius, Rgba8(0, 255, 255), DEBUG_LINE_THICKNESS );
	g_renderer->DrawRing2D( m_position, m_cosmeticRadius, Rgba8(255, 0, 255), DEBUG_LINE_THICKNESS );
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

	g_game->AddScreenShakeIntensity(.05f);
}


//-----------------------------------------------------------------------------------------------
void Entity::TurnToward( float goalOrientationDegrees, float deltaSeconds )
{
	float deltaDegrees = m_maxTurnSpeed * deltaSeconds;
	
	m_orientationDegrees = GetTurnedToward( m_orientationDegrees, goalOrientationDegrees, deltaDegrees );
}


//-----------------------------------------------------------------------------------------------
void Entity::ApplyFriction()
{
	if ( m_velocity.GetLength() > PHYSICS_FRICTION_FRACTION )
	{
		m_velocity -= m_velocity * PHYSICS_FRICTION_FRACTION;
	}
	else
	{
		m_velocity = Vec2( 0.f, 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::UpdateHitFlash( float deltaSeconds )
{
	if ( m_hitFlashTimer > 0.f )
	{
		m_hitFlashTimer -= deltaSeconds;
	}
	else
	{
		m_hitFlashTimer = 0.f;
	}
}

#include "Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
Entity::Entity( Map* map, EntityType type, EntityFaction faction, const Vec2& position ) 
	: m_map( map )
	, m_type( type )
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
	m_position += m_velocity * m_movementSpeedFraction * deltaSeconds;

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
	g_renderer->BindTexture( 0, nullptr );
	DrawRing2D( g_renderer, m_position, m_physicsRadius, Rgba8::CYAN, DEBUG_LINE_THICKNESS );
	DrawRing2D( g_renderer, m_position, m_cosmeticRadius, Rgba8::MAGENTA, DEBUG_LINE_THICKNESS );
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


//-----------------------------------------------------------------------------------------------
bool Entity::CanSeeTarget( const Entity& target, float maxDistance ) const
{
	float distanceToTarget = GetDistance2D( m_position, target.GetPosition() );

	return ( distanceToTarget < maxDistance
			 && m_map->HasLineOfSight( *this, target ) );
}


//-----------------------------------------------------------------------------------------------
bool Entity::CanSeeLivingTarget( const Entity* target, float maxDistance ) const
{
	if ( target == nullptr 
		 || target->IsDead())
	{
		return false;
	}
	
	return CanSeeTarget( *target, maxDistance );
}


//-----------------------------------------------------------------------------------------------
void Entity::RenderHealthBar() const
{
	float healthPercentage = (float)m_health / (float)m_maxHealth;

	g_renderer->BindTexture( 0, nullptr );

	// Black background
	std::vector<Vertex_PCU> backgroundVertexes;
	AABB2 backgroundBox( Vec2::ZERO, Vec2( 1.f, .09f ) );
	Vec2 backgroundPos( m_position.x - .5f, m_position.y + .5f );

	AppendVertsForAABB2D( backgroundVertexes, backgroundBox, Rgba8::BLACK, Vec2::ZERO, Vec2::ONE );
	Vertex_PCU::TransformVertexArray( backgroundVertexes, 1.f, 0.f, backgroundPos );

	g_renderer->DrawVertexArray( backgroundVertexes );

	// Foreground health bar
	std::vector<Vertex_PCU> healthVertexes;
	AABB2 healthBox( Vec2::ZERO, Vec2( healthPercentage * 1.f, .09f ) );
	Vec2 healthPos( m_position.x - .5f, m_position.y + .5f );

	Rgba8 color( Rgba8::GREEN );
	if ( healthPercentage <= .25f )
	{
		color = Rgba8::RED;
	}
	else if ( healthPercentage <= .5f )
	{
		color = Rgba8::YELLOW;
	}

	AppendVertsForAABB2D( healthVertexes, healthBox, color, Vec2::ZERO, Vec2::ONE );
	Vertex_PCU::TransformVertexArray( healthVertexes, 1.f, 0.f, healthPos );

	g_renderer->DrawVertexArray( healthVertexes );
}

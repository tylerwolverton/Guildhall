#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Collider2D.hpp"


//-----------------------------------------------------------------------------------------------
Rigidbody2D::Rigidbody2D( float mass )
{
	GUARANTEE_OR_DIE( mass > 0.f, "Mass must be positive" );
	m_mass = mass;
	m_inverseMass = 1.f / m_mass;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::Update( float deltaSeconds )
{
	// TODO: Make gravity work like real gravity
	Vec2 acceleration = m_forces * m_inverseMass;
	m_velocity += acceleration * deltaSeconds;
	m_worldPosition += m_velocity * deltaSeconds;
	m_collider->UpdateWorldShape();

	m_forces = Vec2::ZERO;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::Destroy()
{
	if ( m_collider != nullptr )
	{
		m_system->DestroyCollider( m_collider );
	}

	m_system->DestroyRigidbody( this );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::TakeCollider( Collider2D* collider )
{
	m_collider = collider;

	m_collider->m_rigidbody = this;
	m_collider->UpdateWorldShape();
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::SetVelocity( const Vec2& velocity )
{
	m_velocity = velocity;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::SetPosition( const Vec2& position )
{
	m_worldPosition = position;

	if ( m_collider != nullptr )
	{
		m_collider->UpdateWorldShape();
	}
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::AddForce( const Vec2& force )
{
	m_forces += force;
}

//-----------------------------------------------------------------------------------------------
void Rigidbody2D::DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	if ( m_collider != nullptr )
	{
		m_collider->DebugRender( renderer, borderColor, fillColor );
	}
}


//-----------------------------------------------------------------------------------------------
Rigidbody2D::~Rigidbody2D()
{
	Destroy();
}

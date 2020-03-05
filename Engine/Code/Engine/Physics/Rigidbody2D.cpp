#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"


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
	if ( !m_isEnabled )
	{
		return;
	}

	Vec2 oldPosition = m_worldPosition;

	Vec2 acceleration = m_forces;
	m_velocity += acceleration * deltaSeconds;
	m_worldPosition += m_velocity * deltaSeconds;
	m_collider->UpdateWorldShape();

	if ( deltaSeconds > -.0001f
		 && deltaSeconds < .0001f )
	{
		m_verletVelocity = Vec2::ZERO;
	}
	else
	{
		m_verletVelocity = ( m_worldPosition - oldPosition ) / deltaSeconds;
	}

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
Vec2 Rigidbody2D::GetImpaceVelocityAtPoint( const Vec2& point )
{
	UNUSED( point );

	return GetVelocity();
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
void Rigidbody2D::Translate2D( const Vec2& translation )
{
	m_worldPosition += translation;

	if ( m_collider != nullptr )
	{
		m_collider->UpdateWorldShape();
	}
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::ChangeMass( float deltaMass )
{
	m_mass += deltaMass;

	ClampMin( m_mass, .001f );

	m_inverseMass = 1.f / m_mass;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::ChangeDrag( float deltaDrag )
{
	m_drag += deltaDrag;

	m_drag = ClampZeroToOne( m_drag );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::AddForce( const Vec2& force )
{
	if ( !m_isEnabled )
	{
		return;
	}

	m_forces += force;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::ApplyImpulseAt( const Vec2& impulse, const Vec2& worldPosition )
{
	UNUSED( worldPosition );

	if ( !m_isEnabled )
	{
		return;
	}

	m_velocity += ( impulse * m_inverseMass );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::ApplyDragForce()
{
	Vec2 dragForce = -GetVerletVelocity() * m_drag;
	AddForce( dragForce );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody2D::DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	Rgba8 rigidbodyColor = m_isEnabled ? Rgba8::BLUE : Rgba8::RED;
	Vec2 crossOffset( .1f, .1f );
	renderer->DrawLine2D( m_worldPosition + crossOffset, m_worldPosition - crossOffset, rigidbodyColor, .03f );
	crossOffset.x *= -1.f;
	renderer->DrawLine2D( m_worldPosition + crossOffset, m_worldPosition - crossOffset, rigidbodyColor, .03f );

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

#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Collider2D.hpp"


//-----------------------------------------------------------------------------------------------
Rigidbody2D::Rigidbody2D()
{

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
void Rigidbody2D::SetPosition( const Vec2& position )
{
	m_worldPosition = position;

	if ( m_collider != nullptr )
	{
		m_collider->UpdateWorldShape();
	}
}


//-----------------------------------------------------------------------------------------------
Rigidbody2D::~Rigidbody2D()
{
	Destroy();
}

#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"


//-----------------------------------------------------------------------------------------------
void Physics2D::BeginFrame()
{

}


//-----------------------------------------------------------------------------------------------
void Physics2D::Update()
{

}


//-----------------------------------------------------------------------------------------------
void Physics2D::EndFrame()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_garbageRigidbodyIndexes.size(); ++rigidbodyIdx )
	{
		Rigidbody2D*& garbageRigidbody = m_rigidbodies[m_garbageRigidbodyIndexes[rigidbodyIdx]];
		delete garbageRigidbody;
		garbageRigidbody = nullptr;
	}

	m_garbageRigidbodyIndexes.clear();

	for ( int colliderIdx = 0; colliderIdx < (int)m_garbageColliderIndexes.size(); ++colliderIdx )
	{
		Collider2D*& garbageCollider = m_colliders[m_garbageColliderIndexes[colliderIdx]];
		delete garbageCollider;
		garbageCollider = nullptr;
	}

	m_garbageColliderIndexes.clear();
}


//-----------------------------------------------------------------------------------------------
Rigidbody2D* Physics2D::CreateRigidbody()
{
	Rigidbody2D* newRigidbody2D = new Rigidbody2D();
	newRigidbody2D->m_system = this;
	m_rigidbodies.push_back( newRigidbody2D );
	
	return newRigidbody2D;
}


//-----------------------------------------------------------------------------------------------
void Physics2D::DestroyRigidbody( Rigidbody2D* rigidbodyToDestroy )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		if ( m_rigidbodies[ rigidbodyIdx ] == rigidbodyToDestroy )
		{
			m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );
			/*delete m_rigidbodies[ rigidbodyIdx ];
			m_rigidbodies[ rigidbodyIdx ] = nullptr;*/
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
DiscCollider2D* Physics2D::CreateDiscCollider( const Vec2& localPosition, float radius )
{
	DiscCollider2D* newCollider2D = new DiscCollider2D( localPosition, radius );
	m_colliders.push_back( newCollider2D );

	return newCollider2D;
}


//-----------------------------------------------------------------------------------------------
void Physics2D::DestroyCollider( Collider2D* colliderToDestroy )
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		if ( m_colliders[ colliderIdx ] == colliderToDestroy )
		{
			m_garbageColliderIndexes.push_back( colliderIdx );
			/*delete m_colliders[ colliderIdx ];
			m_colliders[ colliderIdx ] = nullptr;*/
			break;
		}
	}
}

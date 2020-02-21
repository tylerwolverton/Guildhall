#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Manifold2.hpp"


//-----------------------------------------------------------------------------------------------
void Physics2D::BeginFrame()
{

}


//-----------------------------------------------------------------------------------------------
void Physics2D::Update( float deltaSeconds )
{
	AdvanceSimulation( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Physics2D::AdvanceSimulation( float deltaSeconds )
{
	ApplyEffectors( deltaSeconds ); 	// apply gravity to all dynamic objects
	MoveRigidbodies( deltaSeconds ); 	// apply an euler step to all rigidbodies, and reset per-frame data
	DetectCollisions( deltaSeconds );	// determine all pairs of intersecting colliders
	ResolveCollisions( deltaSeconds ); 	// resolve all collisions, firing appropraite events
	CleanupDestroyedObjects();  		// destroy objects 
}


//-----------------------------------------------------------------------------------------------
void Physics2D::ApplyEffectors( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody2D*& rigidbody = m_rigidbodies[rigidbodyIdx];
		if ( rigidbody != nullptr )
		{
			switch ( rigidbody->GetSimulationMode() )
			{
				case SIMULATION_MODE_DYNAMIC:
				{
					rigidbody->AddForce( m_forceOfGravity );
				}
				break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Physics2D::MoveRigidbodies( float deltaSeconds )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody2D*& rigidbody = m_rigidbodies[rigidbodyIdx];
		if ( rigidbody != nullptr )
		{
			switch ( rigidbody->GetSimulationMode() )
			{
				case SIMULATION_MODE_DYNAMIC:
				case SIMULATION_MODE_KINEMATIC:
				{
					rigidbody->Update( deltaSeconds );
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Physics2D::DetectCollisions( float deltaSeconds )
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		Collider2D* collider = m_colliders[colliderIdx];
		if ( collider == nullptr )
		{
			continue;
		}
		
		// Check intersection with other game objects
		for ( int otherColliderIdx = colliderIdx; otherColliderIdx < (int)m_colliders.size(); ++otherColliderIdx )
		{
			Collider2D* otherCollider = m_colliders[otherColliderIdx];
			if ( collider->Intersects( otherCollider ) )
			{
				Collision2D collision;
				collision.m_myCollider = collider;
				collision.m_theirCollider = otherCollider;
				collision.m_collisionManifold = collider->GetCollisionManifold( otherCollider );

				m_collisions.push_back( collision );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Physics2D::ResolveCollisions( float deltaSeconds )
{
	for ( int collisionIdx = 0; collisionIdx < (int)m_collisions.size(); ++collisionIdx )
	{
		ResolveCollision( m_collisions[collisionIdx] );
	}

	m_collisions.clear();
}


//-----------------------------------------------------------------------------------------------
void Physics2D::ResolveCollision( const Collision2D& collision )
{
	eSimulationMode mySimulationMode = collision.m_myCollider->m_rigidbody->GetSimulationMode();
	eSimulationMode theirSimulationMode = collision.m_theirCollider->m_rigidbody->GetSimulationMode();

	// Do nothing when both are static
	if ( mySimulationMode == SIMULATION_MODE_STATIC
		 && theirSimulationMode == SIMULATION_MODE_STATIC )
	{
		return;
	}

	// Set mass of static and kinematic to infinite and then add impulse

	/*switch ( mySimulationMode )
	{
		case SIMULATION_MODE_STATIC:
			switch ( theirSimulationMode )
			{
				case SIMULATION_MODE_STATIC: return;
				case SIMULATION_MODE_KINEMATIC: 
				case SIMULATION_MODE_DYNAMIC: ApplyAllForceToThem(); return;
			}
			break;

		case SIMULATION_MODE_KINEMATIC:
			switch ( theirSimulationMode )
			{
				case SIMULATION_MODE_STATIC: ApplyAllForceToMe(); return;
				case SIMULATION_MODE_KINEMATIC: ApplyForceToBoth(); return;
				case SIMULATION_MODE_DYNAMIC: ApplyAllForceToThem(); return;
			}
			break;

		case SIMULATION_MODE_DYNAMIC:
			switch ( theirSimulationMode )
			{
				case SIMULATION_MODE_STATIC: 
				case SIMULATION_MODE_KINEMATIC: ApplyAllForceToMe(); return;
				case SIMULATION_MODE_DYNAMIC: ApplyForceToBoth(); return;
			}
			break;
	}*/
}


//-----------------------------------------------------------------------------------------------
void Physics2D::CleanupDestroyedObjects()
{
	// Cleanup rigidbodies
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_garbageRigidbodyIndexes.size(); ++rigidbodyIdx )
	{
		Rigidbody2D*& garbageRigidbody = m_rigidbodies[m_garbageRigidbodyIndexes[rigidbodyIdx]];
		delete garbageRigidbody;
		garbageRigidbody = nullptr;
	}

	m_garbageRigidbodyIndexes.clear();

	// Cleanup colliders
	for ( int colliderIdx = 0; colliderIdx < (int)m_garbageColliderIndexes.size(); ++colliderIdx )
	{
		Collider2D*& garbageCollider = m_colliders[m_garbageColliderIndexes[colliderIdx]];
		delete garbageCollider;
		garbageCollider = nullptr;
	}

	m_garbageColliderIndexes.clear();
}


//-----------------------------------------------------------------------------------------------
void Physics2D::EndFrame()
{
	
}


//-----------------------------------------------------------------------------------------------
Rigidbody2D* Physics2D::CreateRigidbody()
{
	Rigidbody2D* newRigidbody2D = new Rigidbody2D( 10.f );
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
PolygonCollider2D* Physics2D::CreatePolygon2Collider( const Polygon2& polygon )
{
	PolygonCollider2D* newCollider2D = new PolygonCollider2D( polygon );
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
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Physics2D::SetSceneGravity( const Vec2& forceOfGravity )
{
	m_forceOfGravity = forceOfGravity;
}


//-----------------------------------------------------------------------------------------------
void Physics2D::SetSceneGravity( float forceOfGravityY )
{
	m_forceOfGravity = Vec2( 0.f, forceOfGravityY );
}

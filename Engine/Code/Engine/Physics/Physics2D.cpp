#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Manifold2.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
void Physics2D::Startup( Clock* gameClock )
{
	m_gameClock = gameClock;
	if ( m_gameClock == nullptr )
	{
		m_gameClock = Clock::GetMaster();
	}

	m_physicsClock = new Clock( m_gameClock );
	m_stepTimer = new Timer( m_physicsClock );

	m_stepTimer->SetSeconds( m_fixedDeltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Physics2D::BeginFrame()
{

}


//-----------------------------------------------------------------------------------------------
void Physics2D::Update()
{
	while ( m_stepTimer->CheckAndDecrement() )
	{
		AdvanceSimulation( m_fixedDeltaSeconds );
	}
}


//-----------------------------------------------------------------------------------------------
void Physics2D::AdvanceSimulation( float deltaSeconds )
{
	ApplyEffectors(); 					// apply gravity to all dynamic objects
	MoveRigidbodies( deltaSeconds ); 	// apply an euler step to all rigidbodies, and reset per-frame data
	DetectCollisions();					// determine all pairs of intersecting colliders
	ResolveCollisions(); 				// resolve all collisions, firing appropraite events
	CleanupDestroyedObjects();  		// destroy objects 
}


//-----------------------------------------------------------------------------------------------
void Physics2D::ApplyEffectors()
{
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
void Physics2D::DetectCollisions()
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		Collider2D* collider = m_colliders[colliderIdx];
		if ( collider == nullptr )
		{
			continue;
		}
		
		// Check intersection with other game objects
		for ( int otherColliderIdx = colliderIdx + 1; otherColliderIdx < (int)m_colliders.size(); ++otherColliderIdx )
		{
			Collider2D* otherCollider = m_colliders[otherColliderIdx];
			// TODO: Remove Intersects check
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
void Physics2D::ResolveCollisions()
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
	Rigidbody2D* rigidbody1 = collision.m_myCollider->m_rigidbody;
	Rigidbody2D* rigidbody2 = collision.m_theirCollider->m_rigidbody;

	GUARANTEE_OR_DIE( rigidbody1 != nullptr, "My Collider doesn't have a rigidbody" );
	GUARANTEE_OR_DIE( rigidbody2 != nullptr, "Their Collider doesn't have a rigidbody" );

	// Do nothing when both are static
	if ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_STATIC
		 && rigidbody2->GetSimulationMode() == SIMULATION_MODE_STATIC )
	{
		return;
	}

	CorrectCollidingRigidbodies( rigidbody1, rigidbody2, collision.m_collisionManifold );
	ApplyCollisionImpulses( rigidbody1, rigidbody2, collision.m_collisionManifold );
}


//-----------------------------------------------------------------------------------------------
void Physics2D::CorrectCollidingRigidbodies( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold )
{
	if ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_STATIC
		 || ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_KINEMATIC && rigidbody2->GetSimulationMode() == SIMULATION_MODE_DYNAMIC ) )
	{
		rigidbody2->Translate2D( collisionManifold.penetrationDepth * collisionManifold.normal );
		return;
	}
	else if ( rigidbody2->GetSimulationMode() == SIMULATION_MODE_STATIC
			  || ( rigidbody2->GetSimulationMode() == SIMULATION_MODE_KINEMATIC && rigidbody1->GetSimulationMode() == SIMULATION_MODE_DYNAMIC ) )
	{
		rigidbody1->Translate2D( collisionManifold.penetrationDepth * -collisionManifold.normal );
		return;
	}

	float sumOfMasses = rigidbody1->GetMass() + rigidbody2->GetMass();
	float rigidbody1CorrectionDist = ( rigidbody2->GetMass() / sumOfMasses ) * collisionManifold.penetrationDepth;
	float rigidbody2CorrectionDist = ( rigidbody1->GetMass() / sumOfMasses ) * collisionManifold.penetrationDepth;

	rigidbody1->Translate2D( rigidbody1CorrectionDist * -collisionManifold.normal );
	rigidbody2->Translate2D( rigidbody2CorrectionDist * collisionManifold.normal );
}


//-----------------------------------------------------------------------------------------------
void Physics2D::ApplyCollisionImpulses( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold )
{
	if ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_STATIC
		 || ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_KINEMATIC && rigidbody2->GetSimulationMode() == SIMULATION_MODE_DYNAMIC ) )
	{
		CalculateImpulseAgainstImmoveableObject( rigidbody2, rigidbody1, collisionManifold.normal );
	}
	else if ( rigidbody2->GetSimulationMode() == SIMULATION_MODE_STATIC
			  || ( rigidbody2->GetSimulationMode() == SIMULATION_MODE_KINEMATIC && rigidbody1->GetSimulationMode() == SIMULATION_MODE_DYNAMIC ) )
	{
		CalculateImpulseAgainstImmoveableObject( rigidbody1, rigidbody2, collisionManifold.normal );
	}
	else
	{
		CalculateImpulseBetweenMoveableObjects( rigidbody1, rigidbody2, collisionManifold.normal );
	}
}


//-----------------------------------------------------------------------------------------------
void Physics2D::CalculateImpulseAgainstImmoveableObject( Rigidbody2D* moveableRigidbody, Rigidbody2D* immoveableRigidbody, const Vec2& collisionNormal )
{
	float massesRatio = moveableRigidbody->GetMass();

	float e = moveableRigidbody->m_collider->GetBounceWith( immoveableRigidbody->m_collider );
	float impulseMagnitude = massesRatio * ( 1.f + e ) * DotProduct2D( moveableRigidbody->GetVelocity(), collisionNormal );

	moveableRigidbody->ApplyImpulseAt( impulseMagnitude * -collisionNormal, Vec2::ZERO );
}


//-----------------------------------------------------------------------------------------------
void Physics2D::CalculateImpulseBetweenMoveableObjects( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Vec2& collisionNormal )
{
	float sumOfMasses = rigidbody1->GetMass() + rigidbody2->GetMass();
	Vec2 initialVelocity1 = rigidbody1->GetVelocity();
	Vec2 initialVelocity2 = rigidbody2->GetVelocity();

	float productOfMasses = rigidbody1->GetMass() * rigidbody2->GetMass();
	float massesRatio = productOfMasses / sumOfMasses;
	Vec2 differenceOfInitialVelocities = initialVelocity2 - initialVelocity1;

	float impulseMagnitude = massesRatio * ( 1.f + rigidbody1->m_collider->GetBounceWith( rigidbody2->m_collider ) ) * DotProduct2D( differenceOfInitialVelocities, collisionNormal );

	rigidbody1->ApplyImpulseAt( impulseMagnitude * collisionNormal, Vec2::ZERO );
	rigidbody2->ApplyImpulseAt( -impulseMagnitude * collisionNormal, Vec2::ZERO );
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
void Physics2D::Shutdown()
{
	delete m_stepTimer;
	m_stepTimer = nullptr;

	delete m_physicsClock;
	m_physicsClock = nullptr;
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

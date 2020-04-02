#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Manifold2.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
static float s_fixedDeltaSeconds = 1.0f / 120.0f;


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

	m_stepTimer->SetSeconds( s_fixedDeltaSeconds );

	g_eventSystem->RegisterEvent( "set_physics_update", "Usage: set_physics_update hz=NUMBER .Set rate of physics update in hz.", eUsageLocation::DEV_CONSOLE, SetPhysicsUpdateRate );
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
		AdvanceSimulation( s_fixedDeltaSeconds );
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
					rigidbody->ApplyDragForce();
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
	Rigidbody2D* myRigidbody = collision.m_myCollider->m_rigidbody;
	Rigidbody2D* theirRigidbody = collision.m_theirCollider->m_rigidbody;

	GUARANTEE_OR_DIE( myRigidbody != nullptr, "My Collider doesn't have a rigidbody" );
	GUARANTEE_OR_DIE( theirRigidbody != nullptr, "Their Collider doesn't have a rigidbody" );

	// Do nothing when both are static
	if ( myRigidbody->GetSimulationMode() == SIMULATION_MODE_STATIC
		 && theirRigidbody->GetSimulationMode() == SIMULATION_MODE_STATIC )
	{
		return;
	}

	CorrectCollidingRigidbodies( myRigidbody, theirRigidbody, collision.m_collisionManifold );
	ApplyCollisionImpulses( myRigidbody, theirRigidbody, collision.m_collisionManifold );
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
	Manifold2 tangentManifold = collisionManifold;
	tangentManifold.normal = collisionManifold.normal.GetRotatedMinus90Degrees();

	/*DebugAddWorldArrow( collisionManifold.GetCenterOfContactEdge(), collisionManifold.GetCenterOfContactEdge() + collisionManifold.normal, Rgba8::BLUE );*/
	/*DebugAddWorldArrow( tangentManifold.GetCenterOfContactEdge(), tangentManifold.GetCenterOfContactEdge() + tangentManifold.normal, Rgba8::GREEN );*/

	// Handle dynamic vs dynamic
	if ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_DYNAMIC
		 && rigidbody2->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
	{
		float normalImpulseMagnitude = CalculateImpulseBetweenMoveableObjects( rigidbody1, rigidbody2, collisionManifold );
		Vec2 impulsePosition1 = GetNearestPointOnLineSegment2D( rigidbody1->GetCenterOfMass(), collisionManifold.contactPoint1, collisionManifold.contactPoint2 );
		Vec2 impulsePosition2 = GetNearestPointOnLineSegment2D( rigidbody2->GetCenterOfMass(), collisionManifold.contactPoint1, collisionManifold.contactPoint2 );

		DebugAddWorldArrow( impulsePosition1, impulsePosition1 - collisionManifold.normal, Rgba8::BLUE );
		DebugAddWorldArrow( impulsePosition2, impulsePosition2 + collisionManifold.normal, Rgba8::BLUE );

		rigidbody1->ApplyImpulseAt( normalImpulseMagnitude * -collisionManifold.normal, impulsePosition1 );
		rigidbody2->ApplyImpulseAt( normalImpulseMagnitude * collisionManifold.normal, impulsePosition2 );

		float tangentImpulseMagnitude = CalculateImpulseBetweenMoveableObjects( rigidbody1, rigidbody2, tangentManifold );
		if ( fabsf( tangentImpulseMagnitude ) > fabsf( normalImpulseMagnitude ) )
		{
			tangentImpulseMagnitude = SignFloat( tangentImpulseMagnitude ) * fabsf( normalImpulseMagnitude );
		}

		rigidbody1->ApplyImpulseAt( tangentImpulseMagnitude * -tangentManifold.normal, tangentManifold.GetCenterOfContactEdge() );
		rigidbody2->ApplyImpulseAt( tangentImpulseMagnitude * tangentManifold.normal, tangentManifold.GetCenterOfContactEdge() );

		return;
	}

	// Handle other valid simulation mode combinations
	Rigidbody2D* immoveableObj = nullptr;
	Rigidbody2D* moveableObj = nullptr;
	Manifold2 normalManifold = collisionManifold;
	if ( ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_STATIC || rigidbody1->GetSimulationMode() == SIMULATION_MODE_KINEMATIC )
		 && rigidbody2->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
	{
		immoveableObj = rigidbody1;
		moveableObj = rigidbody2;
	}
	else if ( ( rigidbody2->GetSimulationMode() == SIMULATION_MODE_STATIC || rigidbody2->GetSimulationMode() == SIMULATION_MODE_KINEMATIC )
			  && rigidbody1->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
	{
		immoveableObj = rigidbody2;
		moveableObj = rigidbody1;

		normalManifold.normal *= -1.f;
		tangentManifold.normal = normalManifold.normal.GetRotatedMinus90Degrees();
	}
	else
	{
		// Don't apply impulse in this situation
		return;
	}

	float impulseMagnitude = CalculateImpulseAgainstImmoveableObject( moveableObj, immoveableObj, normalManifold );
	Vec2 impulsePosition = GetNearestPointOnLineSegment2D( moveableObj->GetCenterOfMass(), normalManifold.contactPoint1, normalManifold.contactPoint2 );
	DebugAddWorldArrow( impulsePosition, impulsePosition + normalManifold.normal, Rgba8::BLUE );

	moveableObj->ApplyImpulseAt( impulseMagnitude * normalManifold.normal, impulsePosition );

	float tangentMagnitude = CalculateImpulseAgainstImmoveableObject( moveableObj, immoveableObj, tangentManifold );
	float friction = rigidbody1->m_collider->GetFrictionWith( rigidbody2->m_collider );
	if ( fabsf( tangentMagnitude ) > fabsf( impulseMagnitude * friction ) )
	{
		tangentMagnitude = SignFloat( tangentMagnitude ) * fabsf( impulseMagnitude * friction );
	}

	moveableObj->ApplyImpulseAt( tangentMagnitude * tangentManifold.normal, tangentManifold.GetCenterOfContactEdge() );
}


//-----------------------------------------------------------------------------------------------
float Physics2D::CalculateImpulseAgainstImmoveableObject( Rigidbody2D* moveableRigidbody, Rigidbody2D* immoveableRigidbody, const Manifold2& collisionManifold )
{
	float e = moveableRigidbody->m_collider->GetBounceWith( immoveableRigidbody->m_collider );

	Vec2 initialVelocity1 = immoveableRigidbody->GetImpaceVelocityAtPoint( collisionManifold.GetCenterOfContactEdge() );
	Vec2 initialVelocity2 = moveableRigidbody->GetImpaceVelocityAtPoint( collisionManifold.GetCenterOfContactEdge() );
	Vec2 differenceOfInitialVelocities = initialVelocity2 - initialVelocity1;
	   
	float numerator = ( 1.f + e ) * DotProduct2D( differenceOfInitialVelocities, -collisionManifold.normal );

	float moveableRigidbodyRotation = GetRotationalThingOverMomentOfInertia( moveableRigidbody, collisionManifold );

	float inverseMassSum = moveableRigidbody->GetInverseMass();

	float denominator = inverseMassSum + moveableRigidbodyRotation;

	float impulseMagnitude = numerator / denominator;
	return impulseMagnitude;
}


//-----------------------------------------------------------------------------------------------
float Physics2D::CalculateImpulseBetweenMoveableObjects( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold )
{
	Vec2 initialVelocity1 = rigidbody1->GetImpaceVelocityAtPoint( collisionManifold.GetCenterOfContactEdge() );
	Vec2 initialVelocity2 = rigidbody2->GetImpaceVelocityAtPoint( collisionManifold.GetCenterOfContactEdge() );
	Vec2 differenceOfInitialVelocities = initialVelocity2 - initialVelocity1;

	float e = rigidbody1->m_collider->GetBounceWith( rigidbody2->m_collider );

	// This algorithm uses normal from b to a, so flip our normal
	float numerator = ( 1.f + e ) * DotProduct2D( differenceOfInitialVelocities, -collisionManifold.normal );

	float rigidbody1Rotational = GetRotationalThingOverMomentOfInertia( rigidbody1, collisionManifold );
	float rigidbody2Rotational = GetRotationalThingOverMomentOfInertia( rigidbody2, collisionManifold );

	float inverseMassSum = rigidbody1->GetInverseMass() + rigidbody2->GetInverseMass();

	float denominator = inverseMassSum + rigidbody1Rotational + rigidbody2Rotational;

	float impulseMagnitude = numerator / denominator;
	return impulseMagnitude;
	
}


//-----------------------------------------------------------------------------------------------
float Physics2D::GetRotationalThingOverMomentOfInertia( Rigidbody2D* rigidbody, const Manifold2& collisionManifold )
{
	Vec2 centerOfMassToContact = collisionManifold.GetCenterOfContactEdge() - rigidbody->GetPosition();

	// This algorithm uses normal from b to a, so flip our normal
	float rigidbodyRotationOverInertia = DotProduct2D( centerOfMassToContact.GetRotated90Degrees(), -collisionManifold.normal );
	
	rigidbodyRotationOverInertia *= rigidbodyRotationOverInertia;

	rigidbodyRotationOverInertia /= rigidbody->GetMomentOfInertia();

	return rigidbodyRotationOverInertia;
}


//-----------------------------------------------------------------------------------------------
void Physics2D::CleanupDestroyedObjects()
{
	// Cleanup rigidbodies
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_garbageRigidbodyIndexes.size(); ++rigidbodyIdx )
	{
		Rigidbody2D*& garbageRigidbody = m_rigidbodies[m_garbageRigidbodyIndexes[rigidbodyIdx]];
		PTR_SAFE_DELETE( garbageRigidbody );
	}

	m_garbageRigidbodyIndexes.clear();

	// Cleanup colliders
	for ( int colliderIdx = 0; colliderIdx < (int)m_garbageColliderIndexes.size(); ++colliderIdx )
	{
		Collider2D*& garbageCollider = m_colliders[m_garbageColliderIndexes[colliderIdx]];
		PTR_SAFE_DELETE( garbageCollider );
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
	PTR_SAFE_DELETE( m_stepTimer );
	PTR_SAFE_DELETE( m_physicsClock );
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


//-----------------------------------------------------------------------------------------------
float Physics2D::GetFixedDeltaSeconds() const
{
	return s_fixedDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void Physics2D::SetFixedDeltaSeconds( float newDeltaSeconds )
{
	s_fixedDeltaSeconds = newDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
bool Physics2D::SetPhysicsUpdateRate( EventArgs* args )
{
	float hz = args->GetValue( "hz", 120.f );

	if ( hz == 0.f )
	{
		s_fixedDeltaSeconds = 0.f;
	}
	else
	{
		s_fixedDeltaSeconds = 1.f / hz;
	}

	return false;
}

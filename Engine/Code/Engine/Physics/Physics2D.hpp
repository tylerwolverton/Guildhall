#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Physics/Collision2D.hpp"

#include <vector>

//-----------------------------------------------------------------------------------------------
struct Manifold2;
class Polygon2;
class Rigidbody2D;
class Collider2D;
class DiscCollider2D;
class PolygonCollider2D;


//-----------------------------------------------------------------------------------------------
class Physics2D
{
public:
	void BeginFrame();
	void Update( float deltaSeconds );
	void EndFrame();

	// factory style create/destroy
	Rigidbody2D* CreateRigidbody();
	void DestroyRigidbody( Rigidbody2D* rigidbodyToDestroy );

	DiscCollider2D* CreateDiscCollider( const Vec2& localPosition, float radius );
	PolygonCollider2D* CreatePolygon2Collider( const Polygon2& polygon );
	void DestroyCollider( Collider2D* colliderToDestroy );

	const Vec2 GetSceneGravity()															{ return m_forceOfGravity; }
	void SetSceneGravity( const Vec2& forceOfGravity );
	void SetSceneGravity( float forceOfGravityY );

private:
	void AdvanceSimulation( float deltaSeconds );
	void ApplyEffectors( float deltaSeconds ); 	
	void MoveRigidbodies( float deltaSeconds ); 	
	void DetectCollisions( float deltaSeconds ); 	
	void ResolveCollisions( float deltaSeconds ); 	
	void ResolveCollision( const Collision2D& collision ); 	
	// TODO: Rename to my and theirs or something else that's clear
	void CorrectCollidingRigidbodies( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold );
	void CalculateImpulseAgainstImmoveableObject( Rigidbody2D* moveableRigidbody, Rigidbody2D* immoveableRigidbody, const Manifold2& collisionManifold );
	void CalculateImpulseBetweenMoveableObjects( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold );

	void CleanupDestroyedObjects();  	


private:
	std::vector<Rigidbody2D*> m_rigidbodies;
	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<Collider2D*> m_colliders;
	std::vector<int> m_garbageColliderIndexes;

	std::vector<Collision2D> m_collisions;

	Vec2 m_forceOfGravity = Vec2( 0.f, -9.8f );
};

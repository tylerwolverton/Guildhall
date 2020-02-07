#pragma once
#include <vector>

//-----------------------------------------------------------------------------------------------
struct Vec2;
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

private:
	void AdvanceSimulation( float deltaSeconds );
	void ApplyEffectors( float deltaSeconds ); 	
	void MoveRigidbodies( float deltaSeconds ); 	
	void CleanupDestroyedObjects();  	


private:
	std::vector<Rigidbody2D*> m_rigidbodies;
	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<Collider2D*> m_colliders;
	std::vector<int> m_garbageColliderIndexes;
};

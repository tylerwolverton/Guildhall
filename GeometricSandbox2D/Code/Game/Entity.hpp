#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Polygon2.hpp"
#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
typedef int EntityId;


//-----------------------------------------------------------------------------------------------
class Entity
{
	friend class Map;
	
public:
	Entity( const Polygon2& polygon );
	virtual ~Entity() {}

	virtual void			Update( float deltaSeconds );
	virtual void			Render( std::vector<Vertex_PCU>& vertices ) const;
	virtual void			Die();
	virtual void			DebugRender( std::vector<Vertex_PCU>& vertices ) const;

	static void				ResetEntityIds();

	EntityId				GetId() const											{ return m_id; }
	Polygon2				GetConvexPolygon() const								{ return m_convexPolygon; }
	const Vec2				GetForwardVector() const;
	const Vec2				GetPosition() const										{ return m_position; }
	void					SetPosition( const Vec2& position );
	void					UpdatePosition( const Vec2& position );
	const float				GetPhysicsRadius() const								{ return m_physicsRadius; }
	void					SetPhysicsRadius( float physicsRadius )					{ m_physicsRadius = physicsRadius; }
	
	//const float			  GetMass() const										{ return m_entityDef.m_mass; }
	const float				GetOrientationDegrees() const							{ return m_orientationDegrees; }
	void					SetOrientationDegrees( float orientationDegrees )		{ m_orientationDegrees = orientationDegrees; }
	//std::string			  GetName() const										{ return m_entityDef.m_name; }
								 
	bool					IsDead() const											{ return m_isDead; }
	bool					IsGarbage() const										{ return m_isGarbage; }
							
	void					ApplyFriction();
	
	void					RotateAboutPoint2D( float degrees, const Vec2& rotationPoint );
	void					ScaleAboutPoint2D( float scaleFactor, const Vec2& scaleOriginPoint );

	bool					IsPositionInside( const Vec2& position );
	RayConvexHullIntersectionResult	GetRayConvexHullIntersectionResult( const Vec2& rayStartPos, const Vec2& rayForwardNormal );

	void					Highlight();
	void					UnHighlight();

	void					MarkAsCollisionTested()									{ m_hasBeenCollisionTestedThisFrame = true; }
	bool					HasBeenCollisionTestedThisFrame() const					{ return m_hasBeenCollisionTestedThisFrame; }

	void					SetLastRayCollisionDetectedNum( int rayNum )			{ m_lastRayCollisionDetected = rayNum; }
	int						GetLastRayCollisionDetectedNum()						{ return m_lastRayCollisionDetected; }

protected:
	// Game state
	EntityId				m_id = -1;
	bool					m_hasBeenCollisionTestedThisFrame = false;
	int						m_lastRayCollisionDetected = 0;
	Polygon2				m_convexPolygon;
	ConvexHull2D			m_convexHull;
	int						m_curHealth = 1;								// how much health is currently remaining on entity
	bool					m_isDead = false;								// whether the Entity is “dead” in the game; affects entity and game logic
	bool					m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()

	// Physics
	Vec2					m_position = Vec2( 0.f, 0.f );					// the Entity’s 2D(x, y) Cartesian origin / center location, in world space
	Vec2					m_velocity = Vec2( 0.f, 0.f );					// the Entity’s linear 2D( x, y ) velocity, in world units per second
	float					m_physicsRadius = 1.f;
	Vec2					m_linearAcceleration = Vec2( 0.f, 0.f );		// the Entity’s signed linear acceleration per second per second
	float					m_orientationDegrees = 0.f;						// the Entity’s forward - facing direction, as an angle in degrees
	float					m_angularVelocity = 0.f;						// the Entity’s signed angular velocity( spin rate ), in degrees per second

	// Visual
	Rgba8					m_color = Rgba8::GREEN;

	static EntityId			s_nextEntityId;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<Entity*> EntityVector;

#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Time/Time.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
EntityId Entity::s_nextEntityId = 0;

//-----------------------------------------------------------------------------------------------
void Entity::ResetEntityIds()
{
	s_nextEntityId = 0;
}


//-----------------------------------------------------------------------------------------------
Entity::Entity( const Polygon2& polygon )
	: m_convexPolygon( polygon )
{
	m_id = s_nextEntityId++;

	m_color.a = 127;

	m_convexHull = m_convexPolygon.GenerateConvexHull();
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_hasBeenCollisionTestedThisFrame = false;
	m_lastRayCollisionDetected = 0;

	// vel += acceleration * dt;
	m_velocity += m_linearAcceleration * deltaSeconds;
	m_linearAcceleration = Vec2( 0.f, 0.f );
	// pos += vel * dt;
	m_position += m_velocity * deltaSeconds;

	//update orientation
	m_orientationDegrees += m_angularVelocity * deltaSeconds;

	ApplyFriction();
}


//-----------------------------------------------------------------------------------------------
void Entity::Render( std::vector<Vertex_PCU>& vertices ) const
{

	/*Rgba8 color = m_color;
	if ( m_hasBeenCollisionTestedThisFrame )
	{
		color = Rgba8::RED;
	}*/

	AppendVertsForPolygon2( vertices, m_convexPolygon.GetPoints(), m_color );
	
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	m_isDead = true;
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender( std::vector<Vertex_PCU>& vertices ) const
{
	AppendVertsForRing2D( vertices, m_position, m_physicsRadius, Rgba8::CYAN, g_debugLineThickness );

	Rgba8 color = m_color;
	if ( m_lastRayCollisionDetected == 9999999 )
	{
		color = Rgba8::RED;
	}

	AppendVertsForPolygon2( vertices, m_convexPolygon.GetPoints(), color );
}


//-----------------------------------------------------------------------------------------------
void Entity::SetPosition( const Vec2& position )
{
	m_position = position;
}


//-----------------------------------------------------------------------------------------------
void Entity::UpdatePosition( const Vec2& position )
{
	m_convexPolygon.Translate2D( position - m_position );
	m_convexHull = m_convexPolygon.GenerateConvexHull();
	m_position = position;
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
void Entity::ApplyFriction()
{
	if ( m_velocity.GetLength() > PHYSICS_FRICTION_FRACTION )
	{
		m_velocity -= m_velocity * PHYSICS_FRICTION_FRACTION;
	}
	else
	{
		m_velocity = Vec2( 0.f, 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::RotateAboutPoint2D( float degrees, const Vec2& rotationPoint )
{
	m_convexPolygon.RotateAboutPoint2D( degrees, rotationPoint );
	m_convexHull = m_convexPolygon.GenerateConvexHull();

	Vec2 dispToRotationPoint = m_position - rotationPoint;
	dispToRotationPoint.RotateDegrees( -degrees );
	m_position = dispToRotationPoint + rotationPoint;
}


//-----------------------------------------------------------------------------------------------
void Entity::ScaleAboutPoint2D( float scaleFactor, const Vec2& scaleOriginPoint )
{
	m_physicsRadius *= scaleFactor;
	if ( m_physicsRadius < .1f )
	{
		m_physicsRadius = .1f;
		return;
	}
	else if ( m_physicsRadius > 12.f )
	{
		m_physicsRadius = 12.f;
		return;
	}

	m_convexPolygon.ScaleAboutPoint2D( scaleFactor, scaleOriginPoint );
	m_convexHull = m_convexPolygon.GenerateConvexHull();


	Vec2 dispToScaleOriginPoint = m_position - scaleOriginPoint;
	dispToScaleOriginPoint.SetLength( dispToScaleOriginPoint.GetLength() * scaleFactor );
	m_position = dispToScaleOriginPoint + scaleOriginPoint;
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsPositionInside( const Vec2& position )
{
	return m_convexPolygon.Contains( position );
}


//-----------------------------------------------------------------------------------------------
RayConvexHullIntersectionResult Entity::GetRayConvexHullIntersectionResult( const Vec2& rayStartPos, const Vec2& rayForwardNormal )
{
	return m_convexHull.GetRayIntersectionResult( rayStartPos, rayForwardNormal );
}


//-----------------------------------------------------------------------------------------------
void Entity::Highlight()
{
	m_color.a = 225;
}


//-----------------------------------------------------------------------------------------------
void Entity::UnHighlight()
{
	m_color.a = 127;
}

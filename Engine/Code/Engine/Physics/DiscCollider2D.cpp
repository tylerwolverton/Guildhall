#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
DiscCollider2D::DiscCollider2D( const Vec2& localPosition, float radius )
	: m_localPosition( localPosition )
	, m_radius( radius )
{
}


//-----------------------------------------------------------------------------------------------
void DiscCollider2D::UpdateWorldShape()
{
	m_worldPosition = m_localPosition;

	if ( m_rigidbody != nullptr )
	{
		m_worldPosition += m_rigidbody->m_worldPosition;
	}
}


//-----------------------------------------------------------------------------------------------
const Vec2 DiscCollider2D::GetClosestPoint( const Vec2& pos ) const
{
	return GetNearestPointOnDisc2D( pos, m_worldPosition, m_radius );
}


//-----------------------------------------------------------------------------------------------
bool DiscCollider2D::Contains( const Vec2& pos ) const
{
	return IsPointInsideDisc( pos, m_worldPosition, m_radius );
}


//-----------------------------------------------------------------------------------------------
bool DiscCollider2D::Intersects( const Collider2D* other ) const
{
	return false;
}


//-----------------------------------------------------------------------------------------------
void DiscCollider2D::DebugRender( const Rgba8& borderColor, const Rgba8& fillColor )
{

}


//-----------------------------------------------------------------------------------------------
DiscCollider2D::~DiscCollider2D()
{

}

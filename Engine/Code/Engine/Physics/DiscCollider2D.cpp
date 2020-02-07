#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"


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
		m_worldPosition += m_rigidbody->GetPosition();
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
	// TODO: Once we have more than one collider type we need to detect which type we have
	DiscCollider2D* otherDisc = (DiscCollider2D*)other;
	return DoDiscsOverlap( m_worldPosition, m_radius, otherDisc->m_worldPosition, otherDisc->m_radius );
}


//-----------------------------------------------------------------------------------------------
unsigned int DiscCollider2D::CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const
{
	unsigned int edges = SCREEN_EDGE_NONE;
	
	Vec2 discMins( m_worldPosition.x - m_radius, m_worldPosition.y - m_radius );
	Vec2 discMaxs( m_worldPosition.x + m_radius, m_worldPosition.y + m_radius );

	if ( checkForCompletelyOffScreen )
	{
		if ( screenBounds.mins.x > discMaxs.x )
		{
			edges |= SCREEN_EDGE_LEFT;
		}
		else if ( screenBounds.maxs.x < discMins.x )
		{
			edges |= SCREEN_EDGE_RIGHT;
		}

		if ( screenBounds.mins.y > discMaxs.y )
		{
			edges |= SCREEN_EDGE_BOTTOM;
		}
		else if ( screenBounds.maxs.y < discMins.y )
		{
			edges |= SCREEN_EDGE_TOP;
		}
	}
	else
	{
		if ( screenBounds.mins.x > discMins.x )
		{
			edges |= SCREEN_EDGE_LEFT;
		}
		else if ( screenBounds.maxs.x < discMaxs.x )
		{
			edges |= SCREEN_EDGE_RIGHT;
		}

		if ( screenBounds.mins.y > discMins.y )
		{
			edges |= SCREEN_EDGE_BOTTOM;
		}
		else if ( screenBounds.maxs.y < discMaxs.y )
		{
			edges |= SCREEN_EDGE_TOP;
		}
	}

	return edges;
}


//-----------------------------------------------------------------------------------------------
void DiscCollider2D::DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	if( renderer == nullptr )
	{
		return;
	}

	renderer->DrawDisc2D( m_worldPosition, m_radius, fillColor );
	renderer->DrawRing2D( m_worldPosition, m_radius, borderColor, .04f );
}


//-----------------------------------------------------------------------------------------------
DiscCollider2D::~DiscCollider2D()
{

}

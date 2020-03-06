#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
PolygonCollider2D::PolygonCollider2D( const std::vector<Vec2>& points )
{
	m_polygon = Polygon2( points );
	m_type = COLLIDER2D_POLYGON;

	GUARANTEE_OR_DIE( m_polygon.IsConvex(), "Polygon collider is not convex");
}


//-----------------------------------------------------------------------------------------------
PolygonCollider2D::PolygonCollider2D( const Polygon2& polygon )
	: m_polygon( polygon )
{
	GUARANTEE_OR_DIE( m_polygon.IsConvex(), "Polygon collider is not convex" );
	m_type = COLLIDER2D_POLYGON;
}


//-----------------------------------------------------------------------------------------------
PolygonCollider2D::~PolygonCollider2D()
{

}


//-----------------------------------------------------------------------------------------------
void PolygonCollider2D::UpdateWorldShape()
{
	m_worldPosition = m_localPosition;

	if ( m_rigidbody != nullptr )
	{
		m_worldPosition += m_rigidbody->GetPosition();
	}

	m_polygon.SetCenterOfMassAndUpdatePoints( m_worldPosition );
}


//-----------------------------------------------------------------------------------------------
const Vec2 PolygonCollider2D::GetClosestPoint( const Vec2& pos ) const
{
	return m_polygon.GetClosestPoint( pos );
}


//-----------------------------------------------------------------------------------------------
bool PolygonCollider2D::Contains( const Vec2& pos ) const
{
	return m_polygon.Contains( pos );
}


//-----------------------------------------------------------------------------------------------
unsigned int PolygonCollider2D::CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const
{
	unsigned int edges = SCREEN_EDGE_NONE;

	const AABB2 polygonBoundingBox = GetWorldBounds();

	if ( checkForCompletelyOffScreen )
	{
		if ( screenBounds.mins.x > polygonBoundingBox.maxs.x )
		{
			edges |= SCREEN_EDGE_LEFT;
		}
		else if ( screenBounds.maxs.x < polygonBoundingBox.mins.x )
		{
			edges |= SCREEN_EDGE_RIGHT;
		}

		if ( screenBounds.mins.y > polygonBoundingBox.maxs.y )
		{
			edges |= SCREEN_EDGE_BOTTOM;
		}
		else if ( screenBounds.maxs.y < polygonBoundingBox.mins.y )
		{
			edges |= SCREEN_EDGE_TOP;
		}
	}
	else
	{
		if ( screenBounds.mins.x > polygonBoundingBox.mins.x )
		{
			edges |= SCREEN_EDGE_LEFT;
		}
		else if ( screenBounds.maxs.x < polygonBoundingBox.maxs.x )
		{
			edges |= SCREEN_EDGE_RIGHT;
		}

		if ( screenBounds.mins.y > polygonBoundingBox.mins.y )
		{
			edges |= SCREEN_EDGE_BOTTOM;
		}
		else if ( screenBounds.maxs.y < polygonBoundingBox.maxs.y )
		{
			edges |= SCREEN_EDGE_TOP;
		}
	}

	return edges;
}


//-----------------------------------------------------------------------------------------------
void PolygonCollider2D::DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	if ( renderer == nullptr )
	{
		return;
	}

	DrawPolygon2( renderer, m_polygon.GetPoints(), fillColor );
	DrawPolygon2Outline( renderer, m_polygon.GetPoints(), borderColor, .04f );
}

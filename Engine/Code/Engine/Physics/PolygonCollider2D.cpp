#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
PolygonCollider2D::PolygonCollider2D( const std::vector<Vec2>& points )
{
	m_polygon = Polygon2( points );
}


//-----------------------------------------------------------------------------------------------
PolygonCollider2D::PolygonCollider2D( const Polygon2& polygon )
	: m_polygon( polygon )
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
}


//-----------------------------------------------------------------------------------------------
const Vec2 PolygonCollider2D::GetClosestPoint( const Vec2& pos ) const
{
	return Vec2::ZERO;
}


//-----------------------------------------------------------------------------------------------
bool PolygonCollider2D::Contains( const Vec2& pos ) const
{
	return m_polygon.Contains( pos );
}


//-----------------------------------------------------------------------------------------------
bool PolygonCollider2D::Intersects( const Collider2D* other ) const
{
	return false;
}


//-----------------------------------------------------------------------------------------------
void PolygonCollider2D::DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	if ( renderer == nullptr )
	{
		return;
	}

	renderer->DrawPolygon2( m_polygon.GetPoints(), fillColor );
	renderer->DrawPolygon2Outline( m_polygon.GetPoints(), borderColor, .04f );
}


//-----------------------------------------------------------------------------------------------
PolygonCollider2D::~PolygonCollider2D()
{

}

#include "Engine/Physics/DiscCollider2D.hpp"


//-----------------------------------------------------------------------------------------------
DiscCollider2D::DiscCollider2D()
{

}


//-----------------------------------------------------------------------------------------------
void DiscCollider2D::UpdateWorldShape() const
{

}


//-----------------------------------------------------------------------------------------------
const Vec2 DiscCollider2D::GetClosestPoint( const Vec2& pos ) const
{
	return m_worldPosition;
}


//-----------------------------------------------------------------------------------------------
bool DiscCollider2D::Contains( const Vec2& pos ) const
{
	return false;
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

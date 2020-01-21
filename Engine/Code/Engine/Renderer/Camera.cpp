#include "Camera.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
void Camera::SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight )
{
	m_bottomLeft = bottomLeft;
	m_topRight = topRight;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetOrthoView( const AABB2& cameraBounds )
{
	m_bottomLeft = cameraBounds.mins;
	m_topRight = cameraBounds.maxs;
}

//-----------------------------------------------------------------------------------------------
void Camera::Translate2D(const Vec2& translation)
{
	m_bottomLeft += translation;
	m_topRight += translation;
}

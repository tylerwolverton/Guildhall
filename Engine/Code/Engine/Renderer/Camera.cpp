#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
void Camera::SetOutputSize( const Vec2& size )
{
	m_outputSize = size;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetPosition( const Vec3& position )
{
	m_position = position;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetProjectionOrthographic( float height, float nearZ, float farZ )
{
	UNUSED( nearZ );
	UNUSED( farZ );

	float aspectRatio = GetAspectRatio();

	m_outputSize.x = aspectRatio * height;
	m_outputSize.y = height;
}


//-----------------------------------------------------------------------------------------------
Vec2 Camera::ClientToWorldPosition( const Vec2& clientPos )
{
	Vec2 worldPos;
	Vec2 outputDimensions = m_outputSize;

	// for now, you have an orthoMin and orthoMax.
	// so...
	// 1. Convert clientPos to pixel coordinates to normalized coordinates (0, 1) 
	//    by range mapping them from your client size to (0,1) in both dimensions.
	// 2. Convert to your ortho position by rangemapping the normalized coordinate 
	//    from (0,1) to (min, max).
	   
	float worldX = RangeMapFloat( 0.f, 1.f, GetOrthoMin().x, GetOrthoMax().x, clientPos.x );
	float worldY = RangeMapFloat( 0.f, 1.f, GetOrthoMin().y, GetOrthoMax().y, clientPos.y );

	worldPos = Vec2( worldX, worldY );

	// note: you could skip and just RangeMap from (clientSpace to orthoSpace), but the additional
	// step is good practice for something coming up in SD2

	// note 2: client space is y-down, but your world space is y-up, be sure to 
	// take this into account

	// These TODOs are notes on things that will change in this function
	// as MP2 & SD2 advance;
	// TODO - take into account render target
	//        clientPos being the pixel location on the texture

	// TODO - use projection matrix to compute this

	// TODO - Support ndc-depth paramater for 3D-coordinates, needed for ray casts.

	// TODO - take into account viewport

	return worldPos;
}


//-----------------------------------------------------------------------------------------------
float Camera::GetAspectRatio() const
{
	if ( m_outputSize.y == 0.f )
	{
		return 0.f;
	}

	return m_outputSize.x / m_outputSize.y;
}


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


////-----------------------------------------------------------------------------------------------
//void Camera::Translate2D(const Vec2& translation)
//{
//	m_bottomLeft += translation;
//	m_topRight += translation;
//}


//-----------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoMin() const
{
	float orthoX = m_position.x - ( m_outputSize.x * .5f );
	float orthoY = m_position.y - ( m_outputSize.y * .5f );

	return Vec2( orthoX, orthoY );
}


//-----------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoMax() const
{
	float orthoX = m_position.x + ( m_outputSize.x * .5f );
	float orthoY = m_position.y + ( m_outputSize.y * .5f );

	return Vec2( orthoX, orthoY );
}

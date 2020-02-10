#include "Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"


//-----------------------------------------------------------------------------------------------
Camera::Camera()
{
	
}


//-----------------------------------------------------------------------------------------------
Camera::~Camera()
{
	delete m_cameraUBO;
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

//-----------------------------------------------------------------------------------------------
void Camera::Translate2D(const Vec2& translation)
{
	m_bottomLeft += translation;
	m_topRight += translation;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetClearMode( unsigned int clearFlags, Rgba8 color, float depth, unsigned int stencil )
{
	UNUSED( depth );
	UNUSED( stencil );

	m_clearMode = clearFlags;
	m_clearColor = color;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetColorTarget( Texture* texture )
{
	m_colorTarget = texture;
}


//-----------------------------------------------------------------------------------------------
Texture* Camera::GetColorTarget() const
{
	return m_colorTarget;
}


//-----------------------------------------------------------------------------------------------
void Camera::UpdateCameraUBO()
{
	CameraData cameraData;
	cameraData.orthoMin = m_bottomLeft; 
	cameraData.orthoMax = m_topRight;

	m_cameraUBO->Update( &cameraData, sizeof( cameraData ), sizeof( cameraData ) );
}

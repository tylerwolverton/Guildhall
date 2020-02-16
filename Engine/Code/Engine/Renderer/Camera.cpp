#include "Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"


//-----------------------------------------------------------------------------------------------
Camera::Camera()
{
	
}


//-----------------------------------------------------------------------------------------------
Camera::~Camera()
{
	delete m_cameraUBO;
	m_cameraUBO = nullptr;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight )
{
	m_bottomLeft = bottomLeft;
	m_topRight = topRight;

	m_projectionMatrix = MakeOrthographicProjectionMatrixD3D( bottomLeft.x, topRight.x, 
															  bottomLeft.y, topRight.y, 
															  0.f, 1.f );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetOrthoView( const AABB2& cameraBounds )
{
	m_bottomLeft = cameraBounds.mins;
	m_topRight = cameraBounds.maxs;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetPosition( const Vec3& position )
{
	m_position = position;
}


//-----------------------------------------------------------------------------------------------
void Camera::Translate( const Vec3& translation )
{
	m_position += translation;
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
	// if buffer is nulllptr
	// cameraUBO = new RenderBuffer()

	CameraData cameraData;
	cameraData.projection = m_projectionMatrix;
	cameraData.view = Mat44::CreateTranslation3D( -m_position );

	m_cameraUBO->Update( &cameraData, sizeof( cameraData ), sizeof( cameraData ) );
	// return m_cameraUBO;
}

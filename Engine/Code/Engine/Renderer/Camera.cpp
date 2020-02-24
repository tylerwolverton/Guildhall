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
	m_transform.SetPosition( position );
}


//-----------------------------------------------------------------------------------------------
void Camera::Translate( const Vec3& translation )
{
	m_transform.Translate( translation );
}


//-----------------------------------------------------------------------------------------------
void Camera::Translate2D(const Vec2& translation)
{
	m_bottomLeft += translation;
	m_topRight += translation;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetPitchRollYawRotation( float pitch, float roll, float yaw )
{
	m_transform.SetRotationFromPitchRollYawDegrees( pitch, roll, yaw );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetProjectionOrthographic( const Vec2& size, float nearZ, float farZ )
//void Camera::SetProjectionOrthographic( float size, float nearZ, float farZ )
{

	if ( size.y == 0.f )
	{
		return;
	}

	//float aspect = size.x / size.y;
	Vec3 mins( m_transform.m_position - Vec3(size.x * .5f, size.y * .5f, 0.f ) );
	Vec3 maxs( m_transform.m_position + Vec3(size.x * .5f, size.y * .5f, 0.f ) );

	m_projectionMatrix = MakeOrthographicProjectionMatrixD3D( mins.x, maxs.x,
															  mins.y, maxs.y,
															  nearZ, farZ );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetProjectionPerspective( float fovDegrees, float nearZClip, float farZClip )
{
	m_projectionMatrix = MakePerspectiveProjectionMatrixD3D( fovDegrees,
															 GetAspectRatio(),
															 nearZClip, farZClip );
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
void Camera::SetDepthStencilTarget( Texture* texture )
{
	m_deptStencilTarget = texture;
}


//-----------------------------------------------------------------------------------------------
Texture* Camera::GetColorTarget() const
{
	return m_colorTarget;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetOutputSize( const Vec2& size )
{
	m_outputSize = size;
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
void Camera::UpdateCameraUBO()
{
	CameraData cameraData;
	cameraData.projection = m_projectionMatrix;
	cameraData.view = m_transform.GetAsMatrix();

	m_cameraUBO->Update( &cameraData, sizeof( cameraData ), sizeof( cameraData ) );
}

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
Camera::Camera()
{
}


//-----------------------------------------------------------------------------------------------
Camera::~Camera()
{
	PTR_SAFE_DELETE( m_cameraUBO );
}


//-----------------------------------------------------------------------------------------------
Vec3 Camera::ClientToWorldPosition( const Vec2& clientPos, float ndcZ ) const
{
	Vec3 ndc = RangeMapVec3( Vec3::ZERO, Vec3::ONE,
							  Vec3( -1.f, -1.f, 0.f ), Vec3::ONE,
							  Vec3( clientPos, ndcZ ) );

	Mat44 proj = GetProjectionMatrix();
	Mat44 worldToClip = proj;
	worldToClip.PushTransform( GetViewMatrix() );

	Mat44 clipToWorld = worldToClip;
	InvertMatrix( clipToWorld );

	Vec4 worldHomogeneous = clipToWorld.TransformHomogeneousPoint3D( Vec4( ndc, 1.f ) );
	Vec3 worldPos = worldHomogeneous.XYZ() / worldHomogeneous.w;

	return worldPos;
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
Vec2 Camera::GetOrthoMin() const
{
	return ClientToWorldPosition( Vec2::ZERO, 0 ).XY();
}


//-----------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoMax() const
{
	return ClientToWorldPosition( Vec2::ONE, 0 ).XY();
}


//-----------------------------------------------------------------------------------------------
void Camera::SetPitchRollYawRotation( float pitch, float roll, float yaw )
{
	pitch = ClampMinMax( pitch, -85.f, 85.f );

	m_transform.SetRotationFromPitchRollYawDegrees( pitch, roll, yaw );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetProjectionOrthographic( float height, float nearZ, float farZ )
{
	float aspect = GetAspectRatio();
	float halfHeight = height * .5f;
	float halfWidth = halfHeight * aspect;

	Vec2 mins( -halfWidth, -halfHeight );
	Vec2 maxs( halfWidth, halfHeight );

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
	UNUSED( stencil );

	m_clearMode = clearFlags;
	m_clearColor = color;
	m_clearDepth = depth;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetColorTarget( Texture* texture )
{
	m_colorTarget = texture;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetDepthStencilTarget( Texture* texture )
{
	m_depthStencilTarget = texture;
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

	Mat44 model = m_transform.GetAsMatrix();
	InvertMatrix( model );

	cameraData.view = model;
	m_viewMatrix = model;

	m_cameraUBO->Update( &cameraData, sizeof( cameraData ), sizeof( cameraData ) );
}

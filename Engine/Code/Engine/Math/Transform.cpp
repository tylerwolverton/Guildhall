#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"


//-----------------------------------------------------------------------------------------------
void Transform::SetPosition( const Vec3& position )
{
	m_position = position;
}


//-----------------------------------------------------------------------------------------------
void Transform::Translate( const Vec3& translation )
{
	m_position += translation;
}


//-----------------------------------------------------------------------------------------------
void Transform::SetOrientationFromPitchRollYawDegrees( float pitch, float roll, float yaw )
{
	pitch = GetRotationInRangeDegrees( pitch, -180.f, 180.f );
	roll = GetRotationInRangeDegrees( roll, -180.f, 180.f );
	yaw = GetRotationInRangeDegrees( yaw, -180.f, 180.f );

	m_orientation = Vec3( pitch, roll, yaw );
}


//-----------------------------------------------------------------------------------------------
void Transform::RotatePitchRollYawDegrees( float pitch, float roll, float yaw )
{
	SetOrientationFromPitchRollYawDegrees( m_orientation.x + pitch, m_orientation.y + roll, m_orientation.z + yaw );
}


//-----------------------------------------------------------------------------------------------
void Transform::SetScale( const Vec3& scale )
{
	m_scale = scale;
}


//-----------------------------------------------------------------------------------------------
void Transform::SetOrientation( const Vec3& rotation )
{
	m_orientation = rotation;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetAsMatrix() const
{
	Mat44 translation = Mat44::CreateTranslation3D( m_position );
	Mat44 rotation = Mat44::CreateXYZRotationDegrees( m_orientation );
	Mat44 scale = Mat44::CreateNonUniformScale3D( m_scale );

	Mat44 model = translation;
	model.PushTransform( rotation );
	model.PushTransform( scale );

	return model;
}


//-----------------------------------------------------------------------------------------------
Vec3 Transform::GetForwardVector() const
{
	Vec3 forwardVec = GetAsMatrix().TransformVector3D( Vec3( 0.f, 0.f, -1.f ) ).GetNormalized();

	return forwardVec;
}

#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Mat44.hpp"
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
void Transform::SetRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw )
{
	pitch = GetRotationInRangeDegrees( pitch, -90.f, 90.f );
	roll = GetRotationInRangeDegrees( roll, -180.f, 180.f );
	yaw = GetRotationInRangeDegrees( yaw, -180.f, 180.f );

	m_rotation = Vec3( pitch, roll, yaw );
}


//-----------------------------------------------------------------------------------------------
void Transform::RotatePitchRollYawDegrees( float pitch, float roll, float yaw )
{
	SetRotationFromPitchRollYawDegrees( m_rotation.x + pitch, m_rotation.y + roll, m_rotation.z + yaw );
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetAsMatrix() const
{
	Mat44 translation = Mat44::CreateTranslation3D( m_position );
	Mat44 rotation = Mat44::CreateXYZRotationDegrees( m_rotation );
	Mat44 scale = Mat44::CreateNonUniformScale3D( m_scale );

	Mat44 model = translation;
	model.PushTransform( rotation );
	model.PushTransform( scale );

	return model;
}


//-----------------------------------------------------------------------------------------------
float Transform::GetRotationInRangeDegrees( float rotationDegrees, float minAngleDegrees, float maxAngleDegrees )
{
	float newRotation = rotationDegrees;

	// Set between -360.f and 360.f
	while ( newRotation > 360.f ) newRotation -= 360.f;
	while ( newRotation < -360.f ) newRotation += 360.f;

	// Clamp within range
	if ( newRotation > maxAngleDegrees ) newRotation = -( 360.f - newRotation );
	if ( newRotation < minAngleDegrees ) newRotation = ( 360.f - newRotation );

	return newRotation;
}

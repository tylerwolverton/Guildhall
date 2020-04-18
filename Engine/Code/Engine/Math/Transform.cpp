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
void Transform::SetRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw )
{
	pitch = GetRotationInRangeDegrees( pitch, -180.f, 180.f );
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
void Transform::SetScale( const Vec3& scale )
{
	m_scale = scale;
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

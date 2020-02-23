#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Mat44.hpp"


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
	m_rotation = Vec3( pitch, roll, yaw );
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetAsMatrix() const
{
	Mat44 rotation = Mat44::CreateXRotationDegrees( m_rotation.x );
					/** Mat44::CreateYRotationDegrees( m_rotation.y )
					* Mat44::CreateZRotationDegrees( m_rotation.z );*/

	Mat44 translation = Mat44::CreateTranslation3D( m_position );
	Mat44 scale = Mat44::CreateNonUniformScale3D( m_scale );

	return Mat44();
}

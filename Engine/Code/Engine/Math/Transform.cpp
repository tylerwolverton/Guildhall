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
void Transform::SetRotationFromPitchYawRollDegrees( float pitch, float yaw, float roll )
{
	m_rotation = Vec3( pitch, yaw, roll );
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetAsMatrix() const
{
	// Flip position to invert camera controls to match what a user would expect
	Mat44 translation = Mat44::CreateTranslation3D( m_position );
	Mat44 rotation = Mat44::CreateXYZRotationDegrees( m_rotation );
	Mat44 scale = Mat44::CreateNonUniformScale3D( m_scale );

	Mat44 model = translation;
	model.AppendTransform( rotation );
	model.AppendTransform( scale );

	return model;
}

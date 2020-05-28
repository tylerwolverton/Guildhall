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
void Transform::SetOrientationFromPitchRollYawDegrees( float pitchDegrees, float rollDegrees, float yawDegrees )
{
	m_pitchDegrees = GetRotationInRangeDegrees( pitchDegrees, -180.f, 180.f );
	m_rollDegrees = GetRotationInRangeDegrees( rollDegrees, -180.f, 180.f );
	m_yawDegrees = GetRotationInRangeDegrees( yawDegrees, -180.f, 180.f );
}


//-----------------------------------------------------------------------------------------------
void Transform::RotatePitchRollYawDegrees( float pitchDegrees, float rollDegrees, float yawDegrees )
{
	SetOrientationFromPitchRollYawDegrees( m_pitchDegrees + pitchDegrees, 
										   m_rollDegrees + rollDegrees, 
										   m_yawDegrees + yawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Transform::SetScale( const Vec3& scale )
{
	m_scale = scale;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetAsMatrix() const
{
	Mat44 translationMatrix = Mat44::CreateTranslation3D( m_position );
	Mat44 rotationMatrix = GetOrientationAsMatrix();
	Mat44 scaleMatrix = Mat44::CreateNonUniformScale3D( m_scale );

	Mat44 modelMatrix = translationMatrix;
	modelMatrix.PushTransform( rotationMatrix );
	modelMatrix.PushTransform( scaleMatrix );

	return modelMatrix;
}


//-----------------------------------------------------------------------------------------------
Vec3 Transform::GetForwardVector() const
{
	Vec3 forwardVec = GetAsMatrix().TransformVector3D( Vec3( 0.f, 0.f, -1.f ) ).GetNormalized();

	return forwardVec;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetOrientationAsMatrix() const
{
	Mat44 rotationMatrix;
	rotationMatrix.PushTransform( Mat44::CreateYRotationDegrees( m_yawDegrees ) );
	rotationMatrix.PushTransform( Mat44::CreateXRotationDegrees( m_pitchDegrees ) );
	rotationMatrix.PushTransform( Mat44::CreateZRotationDegrees( m_rollDegrees ) );

	return rotationMatrix;
}


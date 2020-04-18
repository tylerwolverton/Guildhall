#pragma once
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
struct Mat44;


//-----------------------------------------------------------------------------------------------
class Transform
{
public:
	void SetPosition( const Vec3& position );
	void SetScale( const Vec3& scale );
	void SetOrientation( const Vec3& rotation );

	void Translate( const Vec3& translation );
	void SetOrientationFromPitchRollYawDegrees( float pitch, float roll, float yaw );
	void RotatePitchRollYawDegrees( float pitch, float roll, float yaw );

	const Mat44 GetAsMatrix() const;

	Vec3 GetPosition() const { return m_position; }
	Vec3 GetForwardVector() const;

public:
	Vec3 m_position		= Vec3::ZERO;
	Vec3 m_orientation	= Vec3::ZERO;
	Vec3 m_scale		= Vec3::ONE;
};
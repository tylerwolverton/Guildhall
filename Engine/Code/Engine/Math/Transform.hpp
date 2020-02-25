#pragma once
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
struct Mat44;


//-----------------------------------------------------------------------------------------------
class Transform
{
public:
	void SetPosition( const Vec3& position );
	void Translate( const Vec3& translation );
	void SetRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw );

	const Mat44 GetAsMatrix() const;

public:
	Vec3 m_position	= Vec3::ZERO;
	Vec3 m_rotation	= Vec3::ZERO;
	Vec3 m_scale	= Vec3::ONE;
};
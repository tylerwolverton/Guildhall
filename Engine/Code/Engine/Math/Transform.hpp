#pragma once
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
struct Mat44;


//-----------------------------------------------------------------------------------------------
enum class eAxisYawPitchRollOrder
{
	YXZ,
	ZXY
};


//-----------------------------------------------------------------------------------------------
struct CoordinateData
{
	eAxisYawPitchRollOrder axisYawPitchRollOrder = eAxisYawPitchRollOrder::YXZ;

	bool invertX = false;
	bool invertY = false;
	bool invertZ = false;
};


//-----------------------------------------------------------------------------------------------
class Transform
{
public:
	void SetPosition( const Vec3& position );
	void SetScale( const Vec3& scale );

	void Translate( const Vec3& translation );
	void SetOrientationFromPitchRollYawDegrees( float pitch, float roll, float yaw );
	void RotatePitchRollYawDegrees( float pitch, float roll, float yaw );

	const Mat44 GetAsMatrix() const;

	Vec3 GetPosition() const { return m_position; }
	Vec3 GetForwardVector() const;

private:
	const Mat44 GetOrientationAsMatrix() const;

public:
	Vec3 m_position = Vec3::ZERO; 
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;
	float m_yawDegrees = 0.f;
	Vec3 m_scale		= Vec3::ONE;
};
#pragma once
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
struct Mat44;


//-----------------------------------------------------------------------------------------------
enum class eAxisYawPitchRollOrder
{
	YXZ,
	ZYX
};


//-----------------------------------------------------------------------------------------------
class AxisOrientation
{
public:
	void InvertXAxis() { m_xAxisDirectionFactor *= -1.f; }
	void InvertYAxis() { m_yAxisDirectionFactor *= -1.f; }
	void InvertZAxis() { m_zAxisDirectionFactor *= -1.f; }

	float GetXAxisDirectionFactor() const { return m_xAxisDirectionFactor; }
	float GetYAxisDirectionFactor() const { return m_yAxisDirectionFactor; }
	float GetZAxisDirectionFactor() const { return m_zAxisDirectionFactor; }

public:
	eAxisYawPitchRollOrder m_axisYawPitchRollOrder = eAxisYawPitchRollOrder::YXZ;

private:
	float m_xAxisDirectionFactor = 1.f;
	float m_yAxisDirectionFactor = 1.f;
	float m_zAxisDirectionFactor = 1.f;
};


//-----------------------------------------------------------------------------------------------
class Transform
{
public:
	static AxisOrientation s_axisOrientation;
	static Mat44 s_identityOrientation;

	void SetPosition( const Vec3& position );
	void SetScale( const Vec3& scale );

	void Translate( const Vec3& translation );
	void SetOrientationFromPitchRollYawDegrees( float pitch, float roll, float yaw );
	void RotatePitchRollYawDegrees( float pitch, float roll, float yaw );

	const Mat44 GetAsMatrix() const;
	const Mat44 GetAsAbsoluteMatrix() const;
	
	const Mat44 GetOrientationAsMatrix() const;
	const Mat44 GetWorldOrientationAsMatrix() const;

	Vec3 GetPosition() const						{ return m_position; }
	Vec3 GetScale() const							{ return m_scale; }
	Vec3 GetForwardVector() const;

	float GetYawDegrees() const						{ return m_yawDegrees; }
	float GetPitchDegrees() const					{ return m_pitchDegrees; }
	float GetRollDegrees() const					{ return m_rollDegrees; }

private:
	Vec3 m_position = Vec3::ZERO; 
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;
	float m_yawDegrees = 0.f;
	Vec3 m_scale		= Vec3::ONE;
};
#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class AnalogJoystick
{
	friend class XboxController;

public:
	explicit AnalogJoystick( float innerDeadZoneFraction, float outerDeadZoneFraction );
	
	Vec2	GetPosition() const					{ return m_correctedPosition; }
	Vec2	GetRawPosition() const				{ return m_rawPosition; }
	float	GetMagnitude() const				{ return m_correctedMagnitude; }
	float	GetDegrees() const					{ return m_correctedDegrees; }
	float	GetInnerDeadZoneFraction() const	{ return m_innerDeadZoneFraction; }
	float	GetOuterDeadZoneFraction() const	{ return m_outerDeadZoneFraction; }
	void	Reset();

private:
	void UpdatePosition( float rawNormalizedX, float rawNormalizedY );

private:
	const float m_innerDeadZoneFraction;
	const float m_outerDeadZoneFraction;
	Vec2		m_rawPosition = Vec2( 0.0f, 0.0f );
	Vec2		m_correctedPosition = Vec2( 0.0f, 0.0f );
	float		m_correctedDegrees = 0.f;
	float		m_correctedMagnitude = 0.f;
};
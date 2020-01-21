#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
AnalogJoystick::AnalogJoystick(float innerDeadZoneFraction, float outerDeadZoneFraction)
	: m_innerDeadZoneFraction(innerDeadZoneFraction)
	, m_outerDeadZoneFraction(outerDeadZoneFraction)
{
}


//-----------------------------------------------------------------------------------------------
void AnalogJoystick::Reset()
{
	m_rawPosition = Vec2( 0.f, 0.f );
	m_correctedPosition = Vec2( 0.f, 0.f );
	m_correctedDegrees = 0.f;
	m_correctedMagnitude = 0.f;
}


//-----------------------------------------------------------------------------------------------
void AnalogJoystick::UpdatePosition( float rawNormalizedX, float rawNormalizedY )
{
	// Save raw position [normalized between (-1, 1)]
	m_rawPosition.x = rawNormalizedX;
	m_rawPosition.y = rawNormalizedY;

	float rawRadius = m_rawPosition.GetLength();

	// Map the radius to the bounds of the dead zone and then normalize between (0, 1)
	float correctedRadius = RangeMapFloat( m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.f, 1.f, rawRadius );
	correctedRadius = ClampMinMax( correctedRadius, 0.f, 1.f );

	// Correct the degrees by normalizing between (0, 360)
	float rawDegrees = m_rawPosition.GetOrientationDegrees();
	m_correctedDegrees = rawDegrees;
	if ( m_correctedDegrees < 0.f )
	{
		m_correctedDegrees += 360.f;
	}

	// Compute corrected position from corrected polar coordinates
	m_correctedPosition.x = correctedRadius * CosDegrees( m_correctedDegrees );
	m_correctedPosition.y = correctedRadius * SinDegrees( m_correctedDegrees );

	m_correctedMagnitude = correctedRadius;
}

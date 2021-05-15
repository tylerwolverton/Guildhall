#pragma once

//-----------------------------------------------------------------------------------------------
struct Vec2;


//-----------------------------------------------------------------------------------------------
// Conversions
float			ConvertDegreesToRadians( float angleDeg );
float			ConvertRadiansToDegrees( float angleRad );

// Trig Functions
float			CosDegrees	( float angleDeg );
float			SinDegrees	( float angleDeg );
float			TanDegrees	( float angleDeg );
float			Atan2Degrees( float y, float x );

// Utilities
int				PositiveMod( int left, int right );
float			SignFloat( float value );
bool			IsNearlyEqual( float value, float target, float variance = .0001f );
bool			IsNearlyEqual( const Vec2& value, const Vec2& target, float variance = .0001f );

// Transforms
const Vec2		TransformPosition2D( const Vec2& initialPos, float scaleUniform, float rotationDeg, const Vec2& translation );
const Vec2		TransformPosition2D( const Vec2& initialPos, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation );

// Distance
float			GetDistance2D		  ( const Vec2& pos1, const Vec2& pos2 );
float			GetDistanceSquared2D  ( const Vec2& pos1, const Vec2& pos2 );

// Direction
const Vec2		GetNormalizedDirectionFromAToB( const Vec2& a, const Vec2& b );

// Intersection Tests
bool			DoDiscsOverlap  ( const Vec2& center1, float radius1, const Vec2& center2, float radius2 );

// Lerp and Clamp
float			Interpolate			( float a, float b, float fractionOfB );
unsigned char	Interpolate			( unsigned char a, unsigned char b, float fractionOfB );
const Vec2		InterpolatePoint2D	( const Vec2& a, const Vec2& b, float fractionOfB );
float			ClampMinMax			( float value, float minimumValue, float maximumValue );
float			ClampMin			( float value, float minimumValue );
double			ClampMinMax			( double value, double minimumValue, double maximumValue );
unsigned char	ClampMinMax			( unsigned char value, unsigned char minimumValue, unsigned char maximumValue );
float			ClampZeroToOne		( float value );
int				ClampMinMaxInt		( int value, int minimumValue, int maximumValue );
float			RangeMapFloat		( float inputBegin, float inputEnd, float outputBegin, float outputEnd, float value );
const Vec2		RangeMapFloatVec2	( float inputBegin, float inputEnd, const Vec2& outputBegin, const Vec2& outputEnd, float value );
int				RoundDownToInt		( float value );

float			GetRotationInRangeDegrees( float rotationDegrees, float minAngleDegrees = -360.f, float maxAngleDegrees = 360.f );

// Geometric Queries 2D
const Vec2		GetNearestPointOnDisc2D			( const Vec2& point, const Vec2& center, float radius );
const Vec2		GetNearestPointInsideDisc2D		( const Vec2& point, const Vec2& center, float radius );
const Vec2		GetNearestPointOnInfiniteLine2D	( const Vec2& point, const Vec2& pointOnLine, const Vec2& anotherPointOnLine );
const Vec2		GetNearestPointOnLineSegment2D	( const Vec2& point, const Vec2& start, const Vec2& end );

bool			DoLineSegmentAndDiscOverlap2D( const Vec2& lineStart, const Vec2& lineForwardNormal, float lineLength, const Vec2& discCenter, float discRadius );

bool			IsPointInsideDisc		( const Vec2& point, const Vec2& center, float radius );
bool			IsPointInsideDiscFast	( const Vec2& point, const Vec2& center, float radius );
bool			IsPointInForwardSector2D( const Vec2& point, const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );

// Turning 2D 
float			GetShortestAngularDisplacementDegrees( float orientationDegreesStart,	float orientationDegreesEnd );
float			GetTurnedToward					 	 ( float currentOrientationDegrees, float goalOrientationDegrees, float deltaDegrees );

// Vector operations
float			DotProduct2D					( const Vec2& a, const Vec2& b );
float			GetProjectedLength2D			( const Vec2& a, const Vec2& b );
const Vec2		GetProjectedOnto2D				( const Vec2& a, const Vec2& onto );
float			GetAngleDegreesBetweenVectors2D	( const Vec2& a, const Vec2& b );

// Easing
float			SmoothStart2( float t);				// [0,1] quadratic ease-in
float			SmoothStart3( float t);				// [0,1] cubic ease-in
float			SmoothStart4( float t);				// [0,1] quartic ease-in
float			SmoothStart5( float t);				// [0,1] quintic ease-in
float			SmoothStop2( float t);				// [0,1] quadratic ease-out
float			SmoothStop3( float t);				// [0,1] cubic ease-out
float			SmoothStop4( float t);				// [0,1] quartic ease-out
float			SmoothStop5( float t);				// [0,1] quintic ease-out
float			SmoothStep3( float t);				// [0,1] cubic ease-in-out

// Templated utils
template < typename T>
T Min( T a, T b )
{
	return a < b ? a : b;
}

template < typename T>
T Max( T a, T b )
{
	return a > b ? a : b;
}

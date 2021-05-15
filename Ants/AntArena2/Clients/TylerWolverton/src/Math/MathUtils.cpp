#include "MathUtils.hpp"
#include "Vec2.hpp"

#include <math.h>


//-----------------------------------------------------------------------------------------------
const float fPI = 3.14159265f;


//-----------------------------------------------------------------------------------------------
float ConvertDegreesToRadians( float angleDeg )
{
	return angleDeg / ( 180.f / fPI );
}


//-----------------------------------------------------------------------------------------------
float ConvertRadiansToDegrees( float angleRad )
{
	return angleRad * ( 180.f / fPI );
}


//-----------------------------------------------------------------------------------------------
float CosDegrees( float angleDeg )
{
	return cosf( ConvertDegreesToRadians( angleDeg ) );
}


//-----------------------------------------------------------------------------------------------
float SinDegrees( float angleDeg )
{
	return sinf( ConvertDegreesToRadians( angleDeg ) );
}


//-----------------------------------------------------------------------------------------------
float TanDegrees( float angleDeg )
{
	return tanf( ConvertDegreesToRadians( angleDeg ) );
}


//-----------------------------------------------------------------------------------------------
float Atan2Degrees( float y, float x )
{
	return ConvertRadiansToDegrees( atan2f( y, x ) );
}


//-----------------------------------------------------------------------------------------------
int PositiveMod( int left, int right )
{
	int modValue = left % right;

	if ( modValue < 0 )
	{
		modValue = right + modValue;
	}

	return modValue;
}


//-----------------------------------------------------------------------------------------------
float SignFloat( float value )
{
	return ( value >= 0.f ) ? 1.f : -1.f;
}


//-----------------------------------------------------------------------------------------------
bool IsNearlyEqual( float value, float target, float variance )
{
	if ( value > target - variance
		 && value < target + variance )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool IsNearlyEqual( const Vec2& value, const Vec2& target, float variance /*= .0001f */ )
{
	if ( ( value.x > target.x - variance
		   && value.x < target.x + variance )
		 && ( value.y > target.y - variance
			  && value.y < target.y + variance ) )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
const Vec2 TransformPosition2D( const Vec2& initialPos, float uniformScale, float rotationDeg, const Vec2& translation )
{
	Vec2 transformedPos( initialPos );

	transformedPos *= uniformScale;
	transformedPos.RotateDegrees( rotationDeg );
	transformedPos += translation;

	return transformedPos;
}


//-----------------------------------------------------------------------------------------------
const Vec2 TransformPosition2D( const Vec2& initialPos, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation )
{
	Vec2 row1( iBasis.x, jBasis.x );
	Vec2 row2( iBasis.y, jBasis.y );
	
	float newX = DotProduct2D( initialPos, row1 );
	float newY = DotProduct2D( initialPos, row2 );

	return Vec2( newX, newY ) + translation;
}


//-----------------------------------------------------------------------------------------------
float GetDistance2D( const Vec2& pos1, const Vec2& pos2 )
{
	return sqrtf( GetDistanceSquared2D(pos1, pos2) );
}


//-----------------------------------------------------------------------------------------------
float GetDistanceSquared2D( const Vec2& pos1, const Vec2& pos2 )
{
	float diffBetweenXCoords = pos2.x - pos1.x;
	float diffBetweenYCoords = pos2.y - pos1.y;

	return ( diffBetweenXCoords * diffBetweenXCoords ) + ( diffBetweenYCoords * diffBetweenYCoords ); 
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNormalizedDirectionFromAToB(const Vec2& a, const Vec2& b)
{
	Vec2 directionAToB = b - a;
	return directionAToB.GetNormalized();
}


//-----------------------------------------------------------------------------------------------
bool DoDiscsOverlap( const Vec2& center1, float radius1, const Vec2& center2, float radius2 )
{
	return GetDistance2D( center1, center2 ) < ( radius1 + radius2 );
}


//-----------------------------------------------------------------------------------------------
float Interpolate( float a, float b, float fractionOfB )
{
	return a + fractionOfB * ( b - a );
}


//-----------------------------------------------------------------------------------------------
unsigned char Interpolate( unsigned char a, unsigned char b, float fractionOfB )
{
	return (unsigned char)( ( 1.f - fractionOfB ) * a ) + (unsigned char)( fractionOfB * b );
}


//-----------------------------------------------------------------------------------------------
const Vec2 InterpolatePoint2D( const Vec2& a, const Vec2& b, float fractionOfB )
{
	// TODO: Optimize this
	Vec2 dir = GetNormalizedDirectionFromAToB( a, b );
	float length = GetDistance2D( a, b ) * fractionOfB;

	return a + ( dir * length );
}


//-----------------------------------------------------------------------------------------------
float RangeMapFloat( float inputBegin, float inputEnd, float outputBegin, float outputEnd, float inputValue )
{
	float inputDisplacement = inputValue - inputBegin;
	float inputRange = inputEnd - inputBegin;
	float fraction = inputDisplacement / inputRange;

	float outputRange = outputEnd - outputBegin;
	float outputDisplacement = fraction * outputRange;

	float outputValue = outputDisplacement + outputBegin;

	return outputValue;
}


//-----------------------------------------------------------------------------------------------
const Vec2 RangeMapFloatVec2( float inputBegin, float inputEnd, const Vec2& outputBegin, const Vec2& outputEnd, float value )
{
	float inputDomain = inputEnd - inputBegin;
	Vec2 outputRange = outputEnd - outputBegin;

	float fraction = ( value - inputBegin ) / inputDomain;

	return outputBegin + ( fraction * outputRange );
}


//-----------------------------------------------------------------------------------------------
int RoundDownToInt( float value )
{
	if ( value < 0.f )
	{
		--value;
	}

	return (int)value;
}


//-----------------------------------------------------------------------------------------------
float GetRotationInRangeDegrees( float rotationDegrees, float minAngleDegrees /*= -360.f*/, float maxAngleDegrees /*= 360.f */ )
{
	float newRotation = rotationDegrees;

	// Set between -360.f and 360.f
	while ( newRotation > 360.f ) newRotation -= 360.f;
	while ( newRotation < -360.f ) newRotation += 360.f;

	// Clamp within range
	if ( newRotation > maxAngleDegrees )
	{
		newRotation = minAngleDegrees + ( newRotation - maxAngleDegrees );
	}
	if ( newRotation < minAngleDegrees )
	{
		newRotation = maxAngleDegrees - ( minAngleDegrees - newRotation );
	}

	return newRotation;
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideDisc( const Vec2& point, const Vec2& center, float radius )
{
	return GetDistance2D( center, point ) < radius;
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideDiscFast( const Vec2& point, const Vec2& center, float radius )
{
	float distSquared = GetDistanceSquared2D( center, point );
	float radiusSquared = radius * radius;

	return distSquared < radiusSquared;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnDisc2D( const Vec2& point, const Vec2& center, float radius )
{
	// The point is inside the disc
	if ( GetDistance2D( center, point ) < radius )
	{
		return point;
	}

	Vec2 directionToPoint = GetNormalizedDirectionFromAToB( center, point );
	return ( directionToPoint * radius ) + center;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointInsideDisc2D( const Vec2& point, const Vec2& center, float radius )
{
	// TODO: Figure out how this is different
	return GetNearestPointOnDisc2D( point, center, radius );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnInfiniteLine2D( const Vec2& point, const Vec2& pointOnLine, const Vec2& anotherPointOnLine )
{
	Vec2 lineDisplacement = anotherPointOnLine - pointOnLine;
	Vec2 localPoint = point - pointOnLine;

	Vec2 projectedPoint = GetProjectedOnto2D( localPoint, lineDisplacement );
	return projectedPoint + pointOnLine;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnLineSegment2D( const Vec2& point, const Vec2& start, const Vec2& end )
{
	Vec2 se = end - start;
	Vec2 sp = point - start;
	Vec2 ep = point - end;
	
	// Region I
	if( DotProduct2D( se, sp ) < 0)
	{
		return start;
	}

	// Region II
	if ( DotProduct2D( se, ep ) > 0 )
	{
		return end;
	}

	// Region III
	return GetNearestPointOnInfiniteLine2D( point, start, end );
}


//-----------------------------------------------------------------------------------------------
bool DoLineSegmentAndDiscOverlap2D( const Vec2& lineStart, const Vec2& lineForwardNormal, float lineLength, const Vec2& discCenter, float discRadius )
{
	Vec2 lineStartToDiscCenter = discCenter - lineStart;

	float lineToDiscProjectedOntoNormal = DotProduct2D( lineForwardNormal, lineStartToDiscCenter );

	if ( lineLength < lineToDiscProjectedOntoNormal - discRadius
		 || lineToDiscProjectedOntoNormal + discRadius < 0.f )
	{
		return false;
	}

	Vec2 tangentToForward = lineForwardNormal.GetRotated90Degrees();

	float lineToDiscProjectedOntoTangent = DotProduct2D( tangentToForward, lineStartToDiscCenter );

	if ( lineToDiscProjectedOntoTangent > discRadius )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
float GetShortestAngularDisplacementDegrees( float orientationDegreesStart, float orientationDegreesEnd )
{
	// Adjust input angles to be between -360 and 360 degrees
	while ( orientationDegreesStart > 360.f )
	{
		orientationDegreesStart -= 360.f;
	}
	while ( orientationDegreesStart < -360.f )
	{
		orientationDegreesStart += 360.f;
	}

	while ( orientationDegreesEnd > 360.f )
	{
		orientationDegreesEnd -= 360.f;
	}
	while ( orientationDegreesEnd < -360.f )
	{
		orientationDegreesEnd += 360.f;
	}

	float angularDisplacementDegrees = orientationDegreesEnd - orientationDegreesStart;
	float angularDisplacementOtherWayDegrees = 360.f - fabsf( angularDisplacementDegrees );

	// Check if other way is shorter
	if ( fabsf( angularDisplacementDegrees ) > fabsf( angularDisplacementOtherWayDegrees ) )
	{
		if ( angularDisplacementDegrees > 0 )
		{
			return angularDisplacementOtherWayDegrees * -1.f;
		}
		else
		{
			return angularDisplacementOtherWayDegrees;
		}
	}

	return angularDisplacementDegrees;
}


//-----------------------------------------------------------------------------------------------
float GetTurnedToward( float currentOrientationDegrees, float goalOrientationDegrees, float deltaDegrees )
{
	float shortestAngle = GetShortestAngularDisplacementDegrees( currentOrientationDegrees, goalOrientationDegrees );

	if ( fabsf( shortestAngle ) < deltaDegrees )
	{
		return goalOrientationDegrees;
	}

	if ( shortestAngle < 0.f )
	{
		return currentOrientationDegrees - deltaDegrees;
	}

	return currentOrientationDegrees + deltaDegrees;
}


//-----------------------------------------------------------------------------------------------
float DotProduct2D( const Vec2& a, const Vec2& b )
{
	return ( a.x*b.x ) + ( a.y*b.y );
}


//-----------------------------------------------------------------------------------------------
float GetProjectedLength2D( const Vec2& a, const Vec2& b )
{
	if ( a == Vec2::ZERO
		 || b == Vec2::ZERO )
	{
		return 0;
	}

	Vec2 normalizedB = b.GetNormalized();
	float length = DotProduct2D( a, normalizedB );

	return length;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetProjectedOnto2D( const Vec2& a, const Vec2& onto )
{
	if ( a == Vec2::ZERO
		 || onto == Vec2::ZERO )
	{
		return Vec2::ZERO;
	}
	
	// Since we aren't normalizing onto, the dot product needs to be divided by the magnitude of onto,
	// then we need to divide onto by its length to get a unit vector in the direction of onto. This results in
	// ( A dot onto ) / onto.length / onto.length or / lengthSquared

	float dotResult = DotProduct2D( a, onto );
	float ontoLengthSquared = onto.GetLengthSquared();

	return ( dotResult * onto ) / ontoLengthSquared;
}


//-----------------------------------------------------------------------------------------------
float GetAngleDegreesBetweenVectors2D( const Vec2& a, const Vec2& b )
{
	return fabsf( GetShortestAngularDisplacementDegrees( a.GetOrientationDegrees(), b.GetOrientationDegrees() ) );
}


//-----------------------------------------------------------------------------------------------
float SmoothStart2( float t )
{
	return t * t;
}


//-----------------------------------------------------------------------------------------------
float SmoothStart3( float t )
{
	return t * t * t;
}


//-----------------------------------------------------------------------------------------------
float SmoothStart4( float t )
{
	return t * t * t * t;
}


//-----------------------------------------------------------------------------------------------
float SmoothStart5( float t )
{
	return t * t * t * t * t;
}


//-----------------------------------------------------------------------------------------------
float SmoothStop2( float t )
{
	float oneMinusT = 1 - t;
	return 1.f - ( oneMinusT * oneMinusT );
}


//-----------------------------------------------------------------------------------------------
float SmoothStop3( float t )
{
	float oneMinusT = 1 - t;
	return 1.f - ( oneMinusT * oneMinusT * oneMinusT );
}


//-----------------------------------------------------------------------------------------------
float SmoothStop4( float t )
{
	float oneMinusT = 1 - t;
	return 1.f - ( oneMinusT * oneMinusT * oneMinusT * oneMinusT );
}


//-----------------------------------------------------------------------------------------------
float SmoothStop5( float t )
{
	float oneMinusT = 1 - t;
	return 1.f - ( oneMinusT * oneMinusT * oneMinusT * oneMinusT * oneMinusT );
}


//-----------------------------------------------------------------------------------------------
float SmoothStep3( float t )
{
	return ( 3 * ( t*t ) ) - ( 2 * ( t*t*t ) );
}


//-----------------------------------------------------------------------------------------------
bool IsPointInForwardSector2D( const Vec2& point, const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist )
{
	Vec2 displacementToTarget = point - observerPos;
	if ( displacementToTarget.GetLengthSquared() > ( maxDist * maxDist ) )
	{
		return false;
	}

	float angleBetweenDegrees = GetAngleDegreesBetweenVectors2D( displacementToTarget, Vec2::MakeFromPolarDegrees( forwardDegrees ) );

	return angleBetweenDegrees < ( apertureDegrees * .5f );
}


//-----------------------------------------------------------------------------------------------
float ClampMinMax( float value, float minimumValue, float maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
double ClampMinMax( double value, double minimumValue, double maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
unsigned char ClampMinMax( unsigned char value, unsigned char minimumValue, unsigned char maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
float ClampMin( float value, float minimumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
float ClampZeroToOne( float value )
{
	return ClampMinMax( value, 0.f, 1.f );
}


//-----------------------------------------------------------------------------------------------
int ClampMinMaxInt( int value, int minimumValue, int maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else
	{
		return value;
	}
}

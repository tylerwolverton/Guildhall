#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <math.h>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
// Static member definitions
const Vec2 Vec2::ZERO( 0.f, 0.f );
const Vec2 Vec2::ONE( 1.f, 1.f );


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
} 


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const IntVec2& intVec2 )
	: x( (float)intVec2.x )
	, y( (float)intVec2.y )
{
}

//------------------------------------------------------------------------------------------------
void Vec2::SetFromText( const char* asText )
{
	std::vector<std::string> splitStrings = SplitStringOnDelimiter( asText, ',' );
	GUARANTEE_OR_DIE( splitStrings.size() == 2, Stringf( "Vec2 can't construct from improper string \"%s\"", asText ) );
	x = (float)atof( splitStrings[0].c_str() );
	y = (float)atof( splitStrings[1].c_str() );
}


//-----------------------------------------------------------------------------------------------
std::string Vec2::GetAsString() const
{
	return ToString();
}


//-----------------------------------------------------------------------------------------------
std::string Vec2::ToString() const
{
	return std::string( Stringf( "( %f, %f )", x, y ) );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator+( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( x * uniformScale, y * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( x * vecToMultiply.x, y * vecToMultiply.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	// TODO: Check for divide by 0 and handle that in a smart way
	float scaleFactor = 1.f / inverseScale;
	return Vec2( x * scaleFactor, y * scaleFactor );
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( float uniformDivisor )
{
	// TODO: Check for divide by 0 and handle that in a smart way
	float scaleFactor = 1.f / uniformDivisor;
	x *= scaleFactor;
	y *= scaleFactor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2( vecToScale.x * uniformScale, vecToScale.y * uniformScale );
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return (x == compare.x && y == compare.y);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return !(x == compare.x && y == compare.y);
}


//-----------------------------------------------------------------------------------------------
float Vec2::GetLength() const
{
	return sqrtf( GetLengthSquared() );
}


//-----------------------------------------------------------------------------------------------
float Vec2::GetLengthSquared() const
{
	return ( x*x ) + ( y*y );
}


//-----------------------------------------------------------------------------------------------
void Vec2::SetLength( float length )
{
	float thetaRad = atan2f( y, x );
	x = length * cosf( thetaRad );
	y = length * sinf( thetaRad );
}


//-----------------------------------------------------------------------------------------------
void Vec2::ClampLength( float maxLength )
{
	if( GetLength() > maxLength )
	{
		SetLength( maxLength );
	}
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetClamped( float maxLength ) const
{
	if( GetLength() <= maxLength )
	{
		return Vec2( x, y );
	}
	else
	{
		Vec2 clampedVec( x, y );
		clampedVec.ClampLength( maxLength );

		return clampedVec;
	}
}


//-----------------------------------------------------------------------------------------------
float Vec2::GetOrientationDegrees() const
{
	return Atan2Degrees( y, x );
}


//-----------------------------------------------------------------------------------------------
float Vec2::GetOrientationRadians() const
{
	return atan2f( y, x );
}


//-----------------------------------------------------------------------------------------------
void Vec2::SetOrientationDegrees( float newOrientationDeg )
{
	float radius = GetLength();
	x = radius * CosDegrees( newOrientationDeg );
	y = radius * SinDegrees( newOrientationDeg );
}


//-----------------------------------------------------------------------------------------------
void Vec2::SetOrientationRadians( float newOrientationRad )
{
	float radius = GetLength();
	x = radius * cosf( newOrientationRad );
	y = radius * sinf( newOrientationRad );
}


//-----------------------------------------------------------------------------------------------
void Vec2::SetPolarDegrees( float angleDeg, float radius /*= 1.f */ )
{
	x = radius * CosDegrees( angleDeg );
	y = radius * SinDegrees( angleDeg );
}


//-----------------------------------------------------------------------------------------------
void Vec2::SetPolarRadians( float angleRad, float radius /*= 1.f */ )
{
	x = radius * cosf( angleRad );
	y = radius * sinf( angleRad );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotatedDegrees( float angleDeg ) const
{
	float radius = GetLength();
	float thetaDeg = Atan2Degrees( y, x );

	thetaDeg += angleDeg;

	return Vec2( radius * CosDegrees( thetaDeg ),
				 radius * SinDegrees( thetaDeg ) );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotatedRadians( float angleRad ) const
{
	float radius = GetLength();
	float thetaRad = atan2f( y, x );

	thetaRad += angleRad;

	return Vec2( radius * cosf( thetaRad ),
				 radius * sinf( thetaRad ) );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotated90Degrees() const
{
	return Vec2( -y, x ); 
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotated90Radians() const
{
	return Vec2( -y, x );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotatedMinus90Degrees() const
{
	return Vec2( y, -x );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetRotatedMinus90Radians() const
{
	return Vec2( y, -x );
}


//-----------------------------------------------------------------------------------------------
void Vec2::RotateDegrees( float angleDeg )
{
	float radius = GetLength();
	float thetaDeg = Atan2Degrees( y, x );

	thetaDeg += angleDeg;

	x = radius * CosDegrees( thetaDeg );
	y = radius * SinDegrees( thetaDeg );
}


//-----------------------------------------------------------------------------------------------
void Vec2::RotateRadians( float angleRad )
{
	float radius = GetLength();
	float thetaRad = atan2f( y, x );

	thetaRad += angleRad;

	x = radius * cosf( thetaRad );
	y = radius * sinf( thetaRad );
}


//-----------------------------------------------------------------------------------------------
void Vec2::Rotate90Degrees()
{
	float oldX = x;
	x = -y;
	y = oldX;
}


//-----------------------------------------------------------------------------------------------
void Vec2::Rotate90Radians()
{
	float oldX = x;
	x = -y;
	y = oldX;
}


//-----------------------------------------------------------------------------------------------
void Vec2::RotateMinus90Degrees()
{
	float oldX = x;
	x = y;
	y = -oldX;
}


//-----------------------------------------------------------------------------------------------
void Vec2::RotateMinus90Radians()
{
	float oldX = x;
	x = y;
	y = -oldX;
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetNormalized() const
{
	float magnitude = GetLength();

	// If the length is 0 this is the zero vector
	if ( magnitude == 0 )
	{
		return Vec2( 0.f, 0.f );
	}

	float inverseMag = 1.f / magnitude;
	return Vec2( x * inverseMag, y * inverseMag );
}


//-----------------------------------------------------------------------------------------------
void Vec2::Normalize()
{
	float magnitude = GetLength();

	// If the length is 0 this is the zero vector, so leave as it is
	if ( magnitude == 0 )
	{
		return;
	}
	
	float inverseMag = 1.f / magnitude;
	x *= inverseMag;
	y *= inverseMag;
}


//-----------------------------------------------------------------------------------------------
float Vec2::NormalizeAndGetPreviousLength()
{
	float magnitude = GetLength();

	// If the length isn't 0, normalize the vector before returning old length
	if ( magnitude != 0 )
	{
		float inverseMag = 1.f / magnitude;
		x *= inverseMag;
		y *= inverseMag;
	}

	return magnitude;
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::GetReflected( const Vec2& normal ) const
{
	Vec2 vN = GetProjectedOnto2D( *this, normal );
	Vec2 vT = *this - vN;
	Vec2 reflected = vT - vN;

	return reflected;
}


//-----------------------------------------------------------------------------------------------
void Vec2::Reflect( const Vec2& normal )
{
	Vec2 vN = GetProjectedOnto2D( *this, normal );
	Vec2 vT = *this - vN;
	Vec2 reflected = vT - vN;

	x = reflected.x;
	y = reflected.y;
}


//-----------------------------------------------------------------------------------------------
Vec2 Vec2::MakeFromPolarDegrees( float angleDeg, float radius /*= 1.f */ )
{
	return Vec2( radius * CosDegrees( angleDeg ),
				 radius * SinDegrees( angleDeg ) );
}


//-----------------------------------------------------------------------------------------------
Vec2 Vec2::MakeFromPolarRadians( float angleRad, float radius /*= 1.f */ )
{
	return Vec2( radius * cosf( angleRad ),
			     radius * sinf( angleRad ) );
}

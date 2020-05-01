#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <math.h>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
// Static member definitions
const Vec3 Vec3::ZERO( 0.f, 0.f, 0.f );
const Vec3 Vec3::ONE( 1.f, 1.f, 1.f );


//-----------------------------------------------------------------------------------------------
Vec3::Vec3( const Vec3& copy )
	: x( copy.x )
	, y( copy.y )
	, z( copy.z )
{
}


//-----------------------------------------------------------------------------------------------
Vec3::Vec3( float initialX, float initialY, float initialZ )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{
}


//-----------------------------------------------------------------------------------------------
Vec3::Vec3( const Vec2& copyFrom, float initialZ )
	: x( copyFrom.x )
	, y( copyFrom.y )
	, z( initialZ )
{

}

//-----------------------------------------------------------------------------------------------
void Vec3::SetFromText( const char* asText )
{
	std::vector<std::string> splitStrings = SplitStringOnDelimiter( asText, ',' );
	GUARANTEE_OR_DIE( splitStrings.size() == 3, Stringf( "Vec3 can't construct from improper string \"%s\"", asText ) );
	x = (float)atof( splitStrings[0].c_str() );
	y = (float)atof( splitStrings[1].c_str() );
	z = (float)atof( splitStrings[2].c_str() );
}


//-----------------------------------------------------------------------------------------------
std::string Vec3::GetAsString() const
{
	return ToString();
}


//-----------------------------------------------------------------------------------------------
std::string Vec3::ToString() const
{
	return std::string( Stringf( "( %f, %f, %f )", x, y, z ) );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator+( const Vec3& vecToAdd ) const
{
	return Vec3( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-( const Vec3& vecToSubtract ) const
{
	return Vec3( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z );
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3( -x, -y, -z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( float uniformScale ) const
{
	return Vec3( x * uniformScale, y * uniformScale, z * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( const Vec3& vecToMultiply ) const
{
	return Vec3( x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/( float inverseScale ) const
{
	// TODO: Check for divide by 0 and handle that in a smart way
	float scaleFactor = 1.f / inverseScale;
	return Vec3( x * scaleFactor, y * scaleFactor, z * scaleFactor );
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=( const Vec3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=( const Vec3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=( float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=( float uniformDivisor )
{
	// TODO: Check for divide by 0 and handle that in a smart way
	float scaleFactor = 1.f / uniformDivisor;
	x *= scaleFactor;
	y *= scaleFactor;
	z *= scaleFactor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=( const Vec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*( float uniformScale, const Vec3& vecToScale )
{
	return Vec3( vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale );
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==( const Vec3& compare ) const
{
	return (x == compare.x && y == compare.y && z == compare.z);
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=( const Vec3& compare ) const
{
	return !(x == compare.x && y == compare.y && z == compare.z);
}


//-----------------------------------------------------------------------------------------------
float Vec3::GetLength() const
{
	return sqrtf( GetLengthSquared() );
}


//-----------------------------------------------------------------------------------------------
float Vec3::GetLengthXY() const
{
	return sqrtf( GetLengthXYSquared() );
}


//-----------------------------------------------------------------------------------------------
float Vec3::GetLengthSquared() const
{
	return ( x*x ) + ( y*y ) + ( z*z );
}


//-----------------------------------------------------------------------------------------------
float Vec3::GetLengthXYSquared() const
{
	return ( x*x ) + ( y*y );
}


//-----------------------------------------------------------------------------------------------
float Vec3::GetOrientationAboutZDegrees() const
{ 
	return Atan2Degrees( y, x );
}


//-----------------------------------------------------------------------------------------------
float Vec3::GetOrientationAboutZRadians() const
{
	return atan2f( y, x );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::GetNormalized() const
{
	float magnitude = GetLength();

	// If the length is 0 this is the zero vector
	if ( magnitude == 0 )
	{
		return Vec3::ZERO;
	}

	float inverseMag = 1.f / magnitude;
	return Vec3( x * inverseMag, y * inverseMag, z * inverseMag );
}


//-----------------------------------------------------------------------------------------------
void Vec3::Normalize()
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
	z *= inverseMag;
}


//-----------------------------------------------------------------------------------------------
Vec3 Vec3::GetRotatedAboutZDegrees( float angleDeg ) const
{
	float radius = GetLengthXY();
	float thetaDeg = Atan2Degrees( y, x );

	thetaDeg += angleDeg;

	return Vec3( radius * CosDegrees( thetaDeg ), 
				 radius * SinDegrees( thetaDeg ), 
				 z );
}


//-----------------------------------------------------------------------------------------------
Vec3 Vec3::GetRotatedAboutZRadians( float angleRad ) const
{
	float radius = GetLengthXY();
	float thetaRad = atan2f( y, x );

	thetaRad += angleRad;

	return Vec3( radius * cosf( thetaRad ),
				 radius * sinf( thetaRad ),
				 z );
}


//-----------------------------------------------------------------------------------------------
Vec2 Vec3::XY() const
{
	return Vec2( x, y );
}


//-----------------------------------------------------------------------------------------------
Vec2 Vec3::XZ() const
{
	return Vec2( x, z );
}

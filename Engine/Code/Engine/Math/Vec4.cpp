#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <math.h>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
Vec4::Vec4( const Vec4& copy )
	: x( copy.x )
	, y( copy.y )
	, z( copy.z )
	, w( copy.w )
{
}


//-----------------------------------------------------------------------------------------------
Vec4::Vec4( float initialX, float initialY, float initialZ, float initialW )
	: x( initialX )
	, y( initialY )
	, z( initialZ )
	, w( initialW )
{
}


//-----------------------------------------------------------------------------------------------
void Vec4::SetFromText( const char* asText )
{
	std::vector<std::string> splitStrings = SplitStringOnDelimiter( asText, ',' );
	GUARANTEE_OR_DIE( splitStrings.size() == 4, Stringf( "Vec4 can't construct from improper string \"%s\"", asText ) );
	x = (float)atof( splitStrings[0].c_str() );
	y = (float)atof( splitStrings[1].c_str() );
	z = (float)atof( splitStrings[2].c_str() );
	w = (float)atof( splitStrings[3].c_str() );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator+( const Vec4& vecToAdd ) const
{
	return Vec4( x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, w + vecToAdd.w );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-( const Vec4& vecToSubtract ) const
{
	return Vec4( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w );
}


//------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-() const
{
	return Vec4( -x, -y, -z, -w );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*( float uniformScale ) const
{
	return Vec4( x * uniformScale, y * uniformScale, z * uniformScale, w * uniformScale );
}


//------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*( const Vec4& vecToMultiply ) const
{
	return Vec4( x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z, w * vecToMultiply.w );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator/( float inverseScale ) const
{
	// TODO: Check for divide by 0 and handle that in a smart way
	float scaleFactor = 1.f / inverseScale;
	return Vec4( x * scaleFactor, y * scaleFactor, z * scaleFactor, w * scaleFactor );
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator+=( const Vec4& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator-=( const Vec4& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator*=( float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator/=( float uniformDivisor )
{
	// TODO: Check for divide by 0 and handle that in a smart way
	float scaleFactor = 1.f / uniformDivisor;
	x *= scaleFactor;
	y *= scaleFactor;
	z *= scaleFactor;
	w *= scaleFactor;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator=( const Vec4& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}


//-----------------------------------------------------------------------------------------------
const Vec4 operator*( float uniformScale, const Vec4& vecToScale )
{
	return Vec4( vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale, vecToScale.w * uniformScale );
}


//-----------------------------------------------------------------------------------------------
bool Vec4::operator==( const Vec4& compare ) const
{
	return ( x == compare.x && y == compare.y && z == compare.z && w == compare.w );
}


//-----------------------------------------------------------------------------------------------
bool Vec4::operator!=( const Vec4& compare ) const
{
	return !( x == compare.x && y == compare.y && z == compare.z && w == compare.w );
}


//-----------------------------------------------------------------------------------------------
float Vec4::GetLength() const
{
	return sqrtf( GetLengthSquared() );
}


//-----------------------------------------------------------------------------------------------
float Vec4::GetLengthSquared() const
{
	return ( x * x ) + ( y * y ) + ( z * z ) + ( w * w );
}

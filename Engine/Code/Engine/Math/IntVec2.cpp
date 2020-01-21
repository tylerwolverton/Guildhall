#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <math.h>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
// Static member definitions
const IntVec2 IntVec2::ZERO = IntVec2( 0, 0 );
const IntVec2 IntVec2::ONE = IntVec2( 1, 1 );


//-----------------------------------------------------------------------------------------------
IntVec2::IntVec2( const IntVec2& copyFrom )
	: x( copyFrom.x )
	, y( copyFrom.y )
{
}


//-----------------------------------------------------------------------------------------------
IntVec2::IntVec2( int initialX, int initialY )
	: x( initialX )
	, y( initialY )
{
}


//-----------------------------------------------------------------------------------------------
void IntVec2::SetFromText( const char* asText )
{
	std::vector<std::string> splitStrings = SplitStringOnDelimiter( asText, ',' );
	GUARANTEE_OR_DIE( splitStrings.size() == 2, Stringf( "IntVec2 can't construct from improper string \"%s\"", asText ) );
	x = atoi( splitStrings[0].c_str() );
	y = atoi( splitStrings[1].c_str() );
}


//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator+( const IntVec2& vecToAdd ) const
{
	return IntVec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator-( const IntVec2& vecToSubtract ) const
{
	return IntVec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator-() const
{
	return IntVec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator*( int uniformScale ) const
{
	return IntVec2( x * uniformScale, y * uniformScale );
}


//-----------------------------------------------------------------------------------------------
const IntVec2 IntVec2::operator*( const IntVec2& vecToMultiply ) const
{
	return IntVec2( x * vecToMultiply.x, y * vecToMultiply.y );
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator*=( int uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
float IntVec2::GetLength() const
{
	return sqrtf( (float)GetLengthSquared() );
}


//-----------------------------------------------------------------------------------------------
int IntVec2::GetLengthSquared() const
{
	return ( x * x ) + ( y * y );
}


//-----------------------------------------------------------------------------------------------
int IntVec2::GetTaxicabLength() const
{
	return abs( x ) + abs( y );
}


//-----------------------------------------------------------------------------------------------
float IntVec2::GetOrientationDegrees() const
{
	return Atan2Degrees( (float)y, (float)x );
}


//-----------------------------------------------------------------------------------------------
float IntVec2::GetOrientationRadians() const
{
	return atan2f( (float)y, (float)x );
}


//-----------------------------------------------------------------------------------------------
IntVec2 IntVec2::GetRotated90Degrees() const
{
	return IntVec2( -y, x );
}


//-----------------------------------------------------------------------------------------------
IntVec2 IntVec2::GetRotated90Radians() const
{
	return IntVec2( -y, x );
}


//-----------------------------------------------------------------------------------------------
IntVec2 IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2( y, -x );
}


//-----------------------------------------------------------------------------------------------
IntVec2 IntVec2::GetRotatedMinus90Radians() const
{
	return IntVec2( y, -x );
}


//-----------------------------------------------------------------------------------------------
void IntVec2::Rotate90Degrees()
{
	int temp = x;
	x = -y;
	y = temp;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::Rotate90Radians()
{
	int temp = x;
	x = -y;
	y = temp;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::RotateMinus90Degrees()
{
	int temp = y;
	y = -x;
	x = temp;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::RotateMinus90Radians()
{
	int temp = y;
	y = -x;
	x = temp;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator=( const IntVec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator-=( const IntVec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void IntVec2::operator+=( const IntVec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
bool IntVec2::operator!=( const IntVec2& compare ) const
{
	return ( x != compare.x || y != compare.y );
}


//-----------------------------------------------------------------------------------------------
bool IntVec2::operator==( const IntVec2& compare ) const
{
	return ( x == compare.x && y == compare.y );
}


//-----------------------------------------------------------------------------------------------
const IntVec2 operator*( int uniformScale, const IntVec2& vecToScale )
{
	return IntVec2( vecToScale.x * uniformScale, vecToScale.y * uniformScale );
}

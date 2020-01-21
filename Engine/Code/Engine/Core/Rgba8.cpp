#include "Rgba8.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <math.h>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
// Static member definitions
const Rgba8 Rgba8::WHITE	( 255, 255, 255 );
const Rgba8 Rgba8::BLACK	( 0, 0, 0 );
const Rgba8 Rgba8::RED		( 255, 0, 0 );
const Rgba8 Rgba8::GREEN	( 0, 255, 0 );
const Rgba8 Rgba8::BLUE		( 0, 0, 255 );
const Rgba8 Rgba8::MAGENTA	( 255, 0, 255 );
const Rgba8 Rgba8::CYAN		( 0, 255, 255 );
const Rgba8 Rgba8::YELLOW	( 255, 255, 0 );
const Rgba8 Rgba8::ORANGE	( 255, 127, 0 );
const Rgba8 Rgba8::PURPLE	( 127, 0, 255 );
const Rgba8 Rgba8::GREY		( 127, 127, 127 );


//-----------------------------------------------------------------------------------------------
Rgba8::Rgba8( unsigned char initialR, unsigned char initialG, unsigned char initialB, unsigned char initialA )
	: r(initialR)
	, g(initialG)
	, b(initialB)
	, a(initialA)
{
}


//-----------------------------------------------------------------------------------------------
void Rgba8::SetFromText( const char* asText )
{
	std::vector<std::string> splitStrings = SplitStringOnDelimiter( asText, ',' );
	GUARANTEE_OR_DIE( splitStrings.size() == 3 || splitStrings.size() == 4, Stringf( "Rgba8 can't construct from improper string \"%s\"", asText ) );

	int rInt = atoi( splitStrings[0].c_str() );
	int gInt = atoi( splitStrings[1].c_str() );
	int bInt = atoi( splitStrings[2].c_str() );
	int aInt = 255;

	if ( splitStrings.size() == 4 )
	{
		aInt = atoi( splitStrings[3].c_str() );
	}

	GUARANTEE_OR_DIE( rInt >= 0 && rInt <=255
					  && gInt >= 0 && gInt <= 255
					  && bInt >= 0 && bInt <= 255
					  && aInt >= 0 && aInt <= 255,
					  Stringf( "Rgba8 can't construct from string with values out of range 0-255 \"%s\"", asText ) );

	r = (unsigned char)rInt;
	g = (unsigned char)gInt;
	b = (unsigned char)bInt;
	a = (unsigned char)aInt;
}


//-----------------------------------------------------------------------------------------------
bool Rgba8::IsRGBEqual( const Rgba8& otherColor )
{
	return r == otherColor.r
		&& g == otherColor.g
		&& b == otherColor.b;
}


//-----------------------------------------------------------------------------------------------
bool Rgba8::operator==( const Rgba8& compare ) const
{
	return r == compare.r 
		&& g == compare.g 
		&& b == compare.b 
		&& a == compare.a;
}


//-----------------------------------------------------------------------------------------------
bool Rgba8::operator!=( const Rgba8& compare ) const
{
	return r != compare.r 
		|| g != compare.g 
		|| b != compare.b 
		|| a != compare.a;
}

#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//-----------------------------------------------------------------------------------------------
IntRange::IntRange( int minAndMax )
	: min( minAndMax )
	, max( minAndMax )
{
}


//-----------------------------------------------------------------------------------------------
IntRange::IntRange( int min, int max )
	: min( min )
	, max( max )
{
}


//-----------------------------------------------------------------------------------------------
IntRange::IntRange( const char* asText )
{
	SetFromText( asText );
}


//-----------------------------------------------------------------------------------------------
bool IntRange::IsInRange( int value ) const
{
	return value >= min && value <= max;
}


//-----------------------------------------------------------------------------------------------
bool IntRange::DoesOverlap( const IntRange& otherRange ) const
{
	if ( min > otherRange.max
		 || max < otherRange.min )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
std::string IntRange::GetAsString() const
{
	return std::string( Stringf( "%i~%i", min, max ) );
}


//-----------------------------------------------------------------------------------------------
int IntRange::GetRandomInRange( RandomNumberGenerator* rng ) const
{
	return rng->RollRandomIntInRange( min, max );
}


//-----------------------------------------------------------------------------------------------
void IntRange::Set( int newMin, int newMax )
{
	min = newMin;
	max = newMax;
}


//-----------------------------------------------------------------------------------------------
void IntRange::SetFromText( const char* asText )
{
	Strings splitStrings = SplitStringOnDelimiter( asText, '~' );
	GUARANTEE_OR_DIE( splitStrings.size() == 1
					  || splitStrings.size() == 2, Stringf( "IntRange can't construct from improper string \"%s\"", asText ) );

	min = atoi( splitStrings[0].c_str() );
	
	if ( splitStrings.size() == 2 )
	{
		max = atoi( splitStrings[1].c_str() );
	}
	else
	{
		max = min;
	}
}

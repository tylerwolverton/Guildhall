#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//-----------------------------------------------------------------------------------------------
FloatRange::FloatRange( float minAndMax )
	: min( minAndMax )
	, max( minAndMax )
{
}


//-----------------------------------------------------------------------------------------------
FloatRange::FloatRange( float min, float max )
	: min( min )
	, max( max )
{
}


//-----------------------------------------------------------------------------------------------
FloatRange::FloatRange( const char* asText )
{
	SetFromText( asText );
}


//-----------------------------------------------------------------------------------------------
bool FloatRange::IsInRange( float value ) const
{
	return value >= min && value <= max;
}


//-----------------------------------------------------------------------------------------------
bool FloatRange::DoesOverlap( const FloatRange& otherRange ) const
{
	if ( min > otherRange.max
		 || max < otherRange.min )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
std::string FloatRange::GetAsString() const
{
	return std::string( Stringf( "%f~%f", min, max ) );
}


//-----------------------------------------------------------------------------------------------
float FloatRange::GetRandomInRange( RandomNumberGenerator* rng ) const
{
	return rng->RollRandomFloatInRange( min, max );
}


//-----------------------------------------------------------------------------------------------
void FloatRange::Set( float newMin, float newMax )
{
	min = newMin;
	max = newMax;
}


//-----------------------------------------------------------------------------------------------
void FloatRange::SetFromText( const char* asText )
{
	Strings splitStrings = SplitStringOnDelimiter( asText, '~' );
	GUARANTEE_OR_DIE( splitStrings.size() == 1
					  || splitStrings.size() == 2, Stringf( "FloatRange can't construct from improper string \"%s\"", asText ) );

	min = (float)atof( splitStrings[0].c_str() );

	if ( splitStrings.size() == 2 )
	{
		max = (float)atof( splitStrings[1].c_str() );
	}
	else
	{
		max = min;
	}
}

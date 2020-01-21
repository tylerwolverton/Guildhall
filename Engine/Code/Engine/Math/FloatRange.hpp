#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class RandomNumberGenerator;


//-----------------------------------------------------------------------------------------------
struct FloatRange
{
public:
	float min = 0.f;
	float max = 1.f;

public:
	FloatRange() = default;
	explicit FloatRange( float minAndMax );			// Construct from single float [N,N]
	explicit FloatRange( float min, float max );	// Construct from [min,max]
	explicit FloatRange( const char* asText );		// Construct from "5.1" or "12.2~19.9"

	// Accessors
	bool		IsInRange( float value ) const;
	bool		DoesOverlap( const FloatRange& otherRange ) const;
	std::string GetAsString() const;
	float		GetRandomInRange( RandomNumberGenerator* rng ) const;

	// Mutators
	void		Set( float newMin, float newMax );
	void		SetFromText( const char* asText ); // Return false if malformatted
};

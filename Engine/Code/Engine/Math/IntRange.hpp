#pragma once
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
class RandomNumberGenerator;


//-----------------------------------------------------------------------------------------------
struct IntRange
{
public:
	int min = 0;
	int max = 0;

public:
	IntRange() = default;
	explicit IntRange( int minAndMax );			// Construct from single int [N,N]
	explicit IntRange( int min, int max );		// Construct from [min,max]
	explicit IntRange( const char* asText );	// Construct from "5" or "12~19"

	// Accessors
	bool				IsInRange( int value ) const;
	bool				DoesOverlap( const IntRange& otherRange ) const;
	std::string			GetAsString() const;
	int					GetRandomInRange( RandomNumberGenerator* rng ) const;
	std::vector<int>	GetAsIntVector() const;

	// Mutators
	void				Set( int newMin, int newMax );
	void				SetFromText( const char* asText ); // Return false if malformatted

	const IntRange operator+( const IntRange& other ) const;
	const IntRange operator-( const IntRange& other ) const;
};

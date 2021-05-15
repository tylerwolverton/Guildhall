#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
struct IntVec2
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	int x = 0;
	int y = 0;

	static const IntVec2 ZERO;
	static const IntVec2 ONE;

public:
	// Construction/Destruction
	~IntVec2() {}												// destructor (do nothing)
	IntVec2() {}												// default constructor (do nothing)
	IntVec2( const IntVec2& copyFrom );							// copy constructor (from another intvec2)
	explicit IntVec2( int initialX, int initialY );				// explicit constructor (from x, y)

	// Operators (const)
	bool			operator==( const IntVec2& compare ) const;			// intvec2 == intvec2
	bool			operator!=( const IntVec2& compare ) const;			// intvec2 != intvec2
	const IntVec2	operator+( const IntVec2& vecToAdd ) const;			// intvec2 + intvec2
	const IntVec2	operator-( const IntVec2& vecToSubtract ) const;	// intvec2 - intvec2
	const IntVec2	operator-() const;									// -intvec2, i.e. "unary negation"
	const IntVec2	operator*( int uniformScale ) const;				// intvec2 * int
	const IntVec2	operator*( const IntVec2& vecToMultiply ) const;	// intvec2 * intvec2

	// Operators (self-mutating / non-const)
	void		operator+=( const IntVec2& vecToAdd );					// intvec2 += intvec2
	void		operator-=( const IntVec2& vecToSubtract );				// intvec2 -= intvec2
	void		operator*=( int uniformScale );							// intvec2 *= int
	void		operator=( const IntVec2& copyFrom );					// intvec2 = intvec2

	// Length getters
	float		GetLength() const;
	int			GetLengthSquared() const;
	int			GetTaxicabLength() const;

	// Angle getters
	float		GetOrientationDegrees() const;
	float		GetOrientationRadians() const;

	// Rotation (return new vectors with const functions)
	IntVec2		GetRotated90Degrees() const;
	IntVec2		GetRotated90Radians() const;
	IntVec2		GetRotatedMinus90Degrees() const;
	IntVec2		GetRotatedMinus90Radians() const;

	// Rotation (self-mutating / non-const)
	void		Rotate90Degrees();
	void		Rotate90Radians();
	void		RotateMinus90Degrees();
	void		RotateMinus90Radians();

	// Standalone "friend" functions that are conceptually, but not actually, part of IntVec2::
	friend const IntVec2 operator*( int uniformScale, const IntVec2& vecToScale );	// int * intvec2
};

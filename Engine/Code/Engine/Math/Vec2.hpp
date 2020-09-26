#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
struct IntVec2;


//-----------------------------------------------------------------------------------------------
struct Vec2
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;

	static const Vec2 ZERO;
	static const Vec2 ONE;
	static const Vec2 HALF;
	static const Vec2 ZERO_TO_ONE;

public:
	// Construction/Destruction
	~Vec2() {}												// destructor (do nothing)
	Vec2() {}												// default constructor (do nothing)
	Vec2( const Vec2& copyFrom );							// copy constructor (from another vec2)
	explicit Vec2( float initialX, float initialY );		// explicit constructor (from x, y)
	explicit Vec2( const IntVec2& intVec2 );				// explicit constructor (from intvec2)
	void SetFromText( const char* asText );
	std::string GetAsString() const;
	std::string ToString() const;

	// Operators (const)
	bool		operator==( const Vec2& compare ) const;		// vec2 == vec2
	bool		operator!=( const Vec2& compare ) const;		// vec2 != vec2
	const Vec2	operator+( const Vec2& vecToAdd ) const;		// vec2 + vec2
	const Vec2	operator-( const Vec2& vecToSubtract ) const;	// vec2 - vec2
	const Vec2	operator-() const;								// -vec2, i.e. "unary negation"
	const Vec2	operator*( float uniformScale ) const;			// vec2 * float
	const Vec2	operator*( const Vec2& vecToMultiply ) const;	// vec2 * vec2
	const Vec2	operator/( float inverseScale ) const;			// vec2 / float

	// Operators (self-mutating / non-const)
	void		operator+=( const Vec2& vecToAdd );				// vec2 += vec2
	void		operator-=( const Vec2& vecToSubtract );		// vec2 -= vec2
	void		operator*=( float uniformScale );				// vec2 *= float
	void		operator/=( float uniformDivisor );				// vec2 /= float
	void		operator=( const Vec2& copyFrom );				// vec2 = vec2

	// Length Manipulation
	float		GetLength() const;
	float		GetLengthSquared() const;
	void		SetLength( float length );
	void		ClampLength( float maxLength );
	const Vec2	GetClamped( float maxLength ) const;

	// Angle Manipulation
	float		GetOrientationDegrees() const;
	float		GetOrientationRadians() const;
	void		SetOrientationDegrees( float newOrientationDeg );
	void		SetOrientationRadians( float newOrientationRad );
	void		SetPolarDegrees( float angleDeg, float radius = 1.f );
	void		SetPolarRadians( float angleRad, float radius = 1.f );

	// Rotation (return new vectors with const functions)
	const Vec2	GetRotatedDegrees( float deltaDeg ) const;
	const Vec2	GetRotatedRadians( float deltaRad ) const;
	const Vec2	GetRotated90Degrees() const;
	const Vec2	GetRotated90Radians() const;
	const Vec2	GetRotatedMinus90Degrees() const;
	const Vec2	GetRotatedMinus90Radians() const;

	// Rotation (self-mutating / non-const)
	void		RotateDegrees( float deltaDeg );
	void		RotateRadians( float deltaRad );
	void		Rotate90Degrees();
	void		Rotate90Radians();
	void		RotateMinus90Degrees();
	void		RotateMinus90Radians();

	// Normalization
	const Vec2	GetNormalized() const;
	void		Normalize();
	float		NormalizeAndGetPreviousLength();

	const Vec2	GetReflected( const Vec2& normal ) const;
	void		Reflect( const Vec2& normal );

	// Vector creation (static)
	static Vec2 MakeFromPolarDegrees( float angleDeg, float radius = 1.f );
	static Vec2 MakeFromPolarRadians( float angleRad, float radius = 1.f );

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend const Vec2 operator*( float uniformScale, const Vec2& vecToScale );	// float * vec2
};

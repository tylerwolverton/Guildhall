#pragma once
//-----------------------------------------------------------------------------------------------
struct Vec2;
struct Vec3;

//-----------------------------------------------------------------------------------------------
struct Vec4
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

public:
	// Construction/Destruction
	~Vec4() {}																			// destructor (do nothing)
	Vec4() {}																			// default constructor (do nothing)
	Vec4( const Vec4& copyFrom );														// copy constructor (from another Vec3)
	explicit Vec4( float initialX, float initialY, float initialZ, float initialW );	// explicit constructor (from x, y, z)
	explicit Vec4( const Vec3& copyFrom, float initialW );	
	explicit Vec4( const Vec2& copyFromXY, const Vec2& copyFromZW );	
	void SetFromText( const char* asText );

	// Operators (const)
	bool		operator==( const Vec4& compare ) const;		// vec4 == vec4
	bool		operator!=( const Vec4& compare ) const;		// vec4 != vec4
	const Vec4	operator+( const Vec4& vecToAdd ) const;		// vec4 + vec4
	const Vec4	operator-( const Vec4& vecToSubtract ) const;	// vec4 - vec4
	const Vec4	operator-() const;								// -vec4, i.e. "unary negation"
	const Vec4	operator*( float uniformScale ) const;			// vec4 * float
	const Vec4	operator*( const Vec4& vecToMultiply ) const;	// vec4 * vec4
	const Vec4	operator/( float inverseScale ) const;			// vec4 / float

	// Operators (self-mutating / non-const)
	void		operator+=( const Vec4& vecToAdd );				// vec4 += vec4
	void		operator-=( const Vec4& vecToSubtract );		// vec4 -= vec4
	void		operator*=( float uniformScale );				// vec4 *= float
	void		operator/=( float uniformDivisor );				// vec4 /= float
	void		operator=( const Vec4& copyFrom );				// vec4 = vec4

	// Length 
	float GetLength() const;
	float GetLengthSquared() const;

	// Accessors
	Vec2 XY() const;
	Vec2 ZW() const;
	Vec3 XYZ() const;

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec4::
	friend const Vec4 operator*( float uniformScale, const Vec4& vecToScale );	// float * vec4
};

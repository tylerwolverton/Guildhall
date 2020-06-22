#pragma once

//-----------------------------------------------------------------------------------------------
struct Rgba8;
struct Vec2;
struct IntVec2;
struct Vec3;
struct Vec4;
struct AABB2;
struct OBB2;
struct Capsule2;
struct FloatRange;


//-----------------------------------------------------------------------------------------------
// Conversions
float			ConvertDegreesToRadians( float angleDeg );
float			ConvertRadiansToDegrees( float angleRad );

// Trig Functions
float			CosDegrees	( float angleDeg );
float			SinDegrees	( float angleDeg );
float			TanDegrees	( float angleDeg );
float			Atan2Degrees( float y, float x );

// Utilities
int				PositiveMod( int left, int right );
float			SignFloat( float value );
bool			IsNearlyEqual( float value, float target, float variance = .0001f );
bool			IsNearlyEqual( const Vec2& value, const Vec2& target, float variance = .0001f );
bool			IsNearlyEqual( const Vec3& value, const Vec3& target, float variance = .0001f );

// Transforms
const Vec2		TransformPosition2D( const Vec2& initialPos, float scaleUniform, float rotationDeg, const Vec2& translation );
const Vec2		TransformPosition2D( const Vec2& initialPos, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation );
const Vec3		TransformPosition3DXY( const Vec3& initialPos, float scaleUniform, float rotationDeg, const Vec2& translation );
const Vec3		TransformPosition3DXY( const Vec3& initialPos, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation );

// Distance
float			GetDistance2D		  ( const Vec2& pos1, const Vec2& pos2 );
float			GetDistanceSquared2D  ( const Vec2& pos1, const Vec2& pos2 );
float			GetDistance3D		  ( const Vec3& pos1, const Vec3& pos2 );
float			GetDistanceSquared3D  ( const Vec3& pos1, const Vec3& pos2 );
float			GetDistanceXY3D	      ( const Vec3& pos1, const Vec3& pos2 );
float			GetDistanceXYSquared3D( const Vec3& pos1, const Vec3& pos2 );
int				GetTaxicabDistance2D  ( const IntVec2& positionA, const IntVec2& positionB ); // aka Manhattan Distance

// Direction
const Vec2		GetNormalizedDirectionFromAToB( const Vec2& a, const Vec2& b );

// Intersection Tests
bool			DoDiscsOverlap  ( const Vec2& center1, float radius1, const Vec2& center2, float radius2 );
bool			DoSpheresOverlap( const Vec3& center1, float radius1, const Vec3& center2, float radius2 );

// Lerp and Clamp
float			Interpolate			( float a, float b, float fractionOfB );
unsigned char	Interpolate			( unsigned char a, unsigned char b, float fractionOfB );
Rgba8			InterpolateColor	( const Rgba8& a, const Rgba8& b, float fractionOfB );
const Vec2		InterpolatePoint2D	( const Vec2& a, const Vec2& b, float fractionOfB );
float			ClampMinMax			( float value, float minimumValue, float maximumValue );
float			ClampMin			( float value, float minimumValue );
double			ClampMinMax			( double value, double minimumValue, double maximumValue );
unsigned char	ClampMinMax			( unsigned char value, unsigned char minimumValue, unsigned char maximumValue );
float			ClampZeroToOne		( float value );
int				ClampMinMaxInt		( int value, int minimumValue, int maximumValue );
float			RangeMapFloat		( float inputBegin, float inputEnd, float outputBegin, float outputEnd, float value );
const Vec2		RangeMapFloatVec2	( float inputBegin, float inputEnd, const Vec2& outputBegin, const Vec2& outputEnd, float value );
const Vec3		RangeMapVec3		( const Vec3& inputBegin, const Vec3& inputEnd, const Vec3& outputBegin, const Vec3& outputEnd, const Vec3& value );
int				RoundDownToInt		( float value );

float			GetRotationInRangeDegrees( float rotationDegrees, float minAngleDegrees = -360.f, float maxAngleDegrees = 360.f );

// Geometric Queries 2D
const Vec2		GetNearestPointOnDisc2D			( const Vec2& point, const Vec2& center, float radius );
const Vec2		GetNearestPointInsideDisc2D		( const Vec2& point, const Vec2& center, float radius );
const Vec2		GetNearestPointOnAABB2D			( const Vec2& point, const AABB2& box );
const Vec2		GetNearestPointOnInfiniteLine2D	( const Vec2& point, const Vec2& pointOnLine, const Vec2& anotherPointOnLine );
const Vec2		GetNearestPointOnLineSegment2D	( const Vec2& point, const Vec2& start, const Vec2& end );
const Vec2		GetNearestPointOnCapsule2D		( const Vec2& point, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius );
const Vec2		GetNearestPointOnCapsule2D		( const Vec2& point, const Capsule2& capsule );
const Vec2		GetNearestPointOnOBB2D			( const Vec2& point, const OBB2& box );

FloatRange		GetRangeOnProjectedAxis     ( int numPoints, const Vec2* points, const Vec2& relativeToPos, const Vec2& axisNormal );
bool			DoOBBAndOBBOverlap2D	    ( const OBB2& boxA, const OBB2& boxB );
bool			DoOBBAndAABBOverlap2D	    ( const OBB2& obb, const AABB2& aabb );
bool			DoOBBAndLineSegmentOverlap2D( const OBB2& obb, const Vec2& lineStart, const Vec2& lineEnd );
bool			DoOBBAndCapsuleOverlap2D	( const OBB2& obb, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius );
bool			DoOBBAndCapsuleOverlap2D	( const OBB2& obb, const Capsule2& capsule );
bool			DoOBBAndDiscOverlap2D	    ( const OBB2& obb, const Vec2& discCenter, float discRadius );

bool			DoAABBsOverlap2D			( const AABB2& box1, const AABB2& box2 );
bool			DoDiscAndAABBOverlap2D		( const Vec2& center, float radius, const AABB2& box );

void			PushDiscOutOfDisc2D		 ( Vec2& mobileCenter,	float mobileRadius, const Vec2& stationaryCenter,	float stationaryRadius );
void			PushDiscsOutOfEachOther2D( Vec2& center1,		float radius1,			  Vec2& center2,			float radius2 );
void			PushDiscOutOfPoint2D	 ( Vec2& center,		float radius,		const Vec2& point );
void			PushDiscOutOfAABB2D		 ( Vec2& center,		float radius,		const AABB2& box );

bool			IsPointInsideDisc		( const Vec2& point, const Vec2& center, float radius );
bool			IsPointInsideAABB2D		( const Vec2& point, const AABB2& box );
bool			IsPointInsideCapsule2D	( const Vec2& point, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius );
bool			IsPointInsideCapsule2D	( const Vec2& point, const Capsule2& capsule );
bool			IsPointInsideOBB2D		( const Vec2& point, const OBB2& box );
bool			IsPointInForwardSector2D( const Vec2& point, const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );

// Turning 2D 
float			GetShortestAngularDisplacementDegrees( float orientationDegreesStart,	float orientationDegreesEnd );
float			GetTurnedToward					 	 ( float currentOrientationDegrees, float goalOrientationDegrees, float deltaDegrees );

// Vector operations
float			DotProduct2D					( const Vec2& a, const Vec2& b );
float			DotProduct3D					( const Vec3& a, const Vec3& b );
float			DotProduct4D					( const Vec4& a, const Vec4& b );
float			GetProjectedLength2D			( const Vec2& a, const Vec2& b );
const Vec2		GetProjectedOnto2D				( const Vec2& a, const Vec2& onto );
float			GetAngleDegreesBetweenVectors2D	( const Vec2& a, const Vec2& b );

Vec3			CrossProduct3D					( const Vec3& a, const Vec3& b );
Vec3			TripleProduct3D					( const Vec3& a, const Vec3& b, const Vec3& c );
Vec2			TripleProduct2D					( const Vec2& a, const Vec2& b, const Vec2& c );

// Easing
float			SmoothStart2( float t);				// [0,1] quadratic ease-in
float			SmoothStart3( float t);				// [0,1] cubic ease-in
float			SmoothStart4( float t);				// [0,1] quartic ease-in
float			SmoothStart5( float t);				// [0,1] quintic ease-in
float			SmoothStop2( float t);				// [0,1] quadratic ease-out
float			SmoothStop3( float t);				// [0,1] cubic ease-out
float			SmoothStop4( float t);				// [0,1] quartic ease-out
float			SmoothStop5( float t);				// [0,1] quintic ease-out
float			SmoothStep3( float t);				// [0,1] cubic ease-in-out

// Templated utils
template < typename T>
T Min( T a, T b )
{
	return a < b ? a : b;
}

template < typename T>
T Max( T a, T b )
{
	return a > b ? a : b;
}

//-----------------------------------------------------------------------------------------------
// UnitTests_Custom.cpp
//
// YOU MAY CHANGE anything in this file.  Make sure your "RunTestSet()" commands at the bottom
//	of the file in RunTest_Custom() all pass "false" for their first argument (isGraded).
//
#include "Game/UnitTests_Custom.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
// Include whatever you want here to test various engine classes (even non-math stuff, like Rgba)

//-----------------------------------------------------------------------------------------------
// Forward declarations for selective helper functions needed here
//
bool IsMostlyEqual( float a, float b, float epsilon = 0.001f );
bool IsMostlyEqual( const Vec2& vec2, float x, float y );
bool IsMostlyEqual( const Vec2& vec2a, const Vec2& vec2b );
bool IsMostlyEqual( const Vec3& vec3a, const Vec3& vec3b );

//-----------------------------------------------------------------------------------------------
int TestSet_Custom_Dummy()
{
	VerifyTestResult( 2 == 2, "Dummy example test that passes (2 == 2)" ); // e.g. a non-graded test that passes
	VerifyTestResult( 2 == 3, "Dummy example test that fails (2 == 3)" ); // e.g. a non-graded test that doesn't pass

	return 2; // Number of tests expected (equal to the # of times you call VerifyTestResult)
}


//-----------------------------------------------------------------------------------------------
int TestSet_Custom_Rgba()
{
	// Write your own tests here; each call to VerifyTestResult is considered a "test".

	return 0; // Number of tests expected (set equal to the # of times you call VerifyTestResult)
}


//-----------------------------------------------------------------------------------------------
int TestSet_Custom_MathUtils()
{
	VerifyTestResult( IsMostlyEqual( Vec2( -2.6f, -5.2f ), GetProjectedOnto2D( Vec2( 3.f, -8.f ), Vec2( 1.f, 2.f ) ) ), "Projected vector should be ( -2.6f, -5.2f )" );
	
	return 1; // Number of tests expected (set equal to the # of times you call VerifyTestResult)
}

//-----------------------------------------------------------------------------------------------
int TestSet_Custom_SetFromText()
{
	Vec2 vec2ToSet;

	vec2ToSet.SetFromText( "," );
	VerifyTestResult( IsMostlyEqual( Vec2( 0.f, 0.f ), vec2ToSet ), "Vector2 should be ( 0.f, 0.f )" );

	vec2ToSet.SetFromText( "1,2" );
	VerifyTestResult( IsMostlyEqual( Vec2( 1.f, 2.f ), vec2ToSet ), "Vector2 should be ( 1.f, 2.f )" );

	vec2ToSet.SetFromText( "-1.01,-2.0" );
	VerifyTestResult( IsMostlyEqual( Vec2( -1.01f, -2.0f ), vec2ToSet ), "Vector2 should be ( -1.01f, -2.0f )" );

	vec2ToSet.SetFromText( "  -1  ,   2 " );
	VerifyTestResult( IsMostlyEqual( Vec2( -1.f, 2.f ), vec2ToSet ), "Vector2 should be ( -1.f, 2.f )" );


	Vec3 vec3ToSet;

	vec3ToSet.SetFromText( ",," );
	VerifyTestResult( IsMostlyEqual( Vec3( 0.f, 0.f, 0.f ), vec3ToSet ), "Vector3 should be ( 0.f, 0.f, 0.f )" );

	vec3ToSet.SetFromText( "1,2,3" );
	VerifyTestResult( IsMostlyEqual( Vec3( 1.f, 2.f, 3.f ), vec3ToSet ), "Vector3 should be ( 1.f, 2.f )" );

	vec3ToSet.SetFromText( "-1.01,-2.0,-3." );
	VerifyTestResult( IsMostlyEqual( Vec3( -1.01f, -2.0f, -3. ), vec3ToSet ), "Vector3 should be ( -1.01f, -2.0f, -3.f )" );

	vec3ToSet.SetFromText( "  -1  ,   2 , 3.5  " );
	VerifyTestResult( IsMostlyEqual( Vec3( -1.f, 2.f, 3.5f ), vec3ToSet ), "Vector3 should be ( -1.f, 2.f, 3.5f )" );


	IntVec2 intVec2ToSet;

	intVec2ToSet.SetFromText( "," );
	VerifyTestResult( IntVec2( 0, 0 ) == intVec2ToSet, "IntVector2 should be ( 0, 0 )" );

	intVec2ToSet.SetFromText( "1,2" );
	VerifyTestResult( IntVec2( 1, 2 ) == intVec2ToSet, "IntVector2 should be ( 1, 2 )" );

	intVec2ToSet.SetFromText( "-1,-2" );
	VerifyTestResult( IntVec2( -1, -2 ) == intVec2ToSet, "IntVector2 should be ( -1, -2 )" );

	intVec2ToSet.SetFromText( "  -1  ,   2 " );
	VerifyTestResult( IntVec2( -1, 2 ) == intVec2ToSet, "IntVector2 should be ( -1, 2 )" );


	Rgba8 rgba8ToSet;

	rgba8ToSet.SetFromText( ",," );
	VerifyTestResult( Rgba8::BLACK == rgba8ToSet, "Rgba8 should be ( 0, 0, 0, 255 )" );

	rgba8ToSet.SetFromText( ",,," );
	VerifyTestResult( Rgba8(0,0,0,0) == rgba8ToSet, "Rgba8 should be ( 0, 0, 0, 0 )" );

	rgba8ToSet.SetFromText( "127,127,127" );
	VerifyTestResult( Rgba8::GREY == rgba8ToSet, "Rgba8 should be ( 127, 127, 127, 255 )" );

	rgba8ToSet.SetFromText( "127,127,127,100" );
	VerifyTestResult( Rgba8( 127, 127, 127, 100 ) == rgba8ToSet, "Rgba8 should be ( 127, 127, 127, 100 )" );

	rgba8ToSet.SetFromText( "127  , 127,   127 " );
	VerifyTestResult( Rgba8::GREY == rgba8ToSet, "Rgba8 should be ( 127, 127, 127, 255 )" );

	return 17; // Number of tests expected (set equal to the # of times you call VerifyTestResult)
}

//-----------------------------------------------------------------------------------------------
void RunTests_Custom()
{
	// Always set first argument to "false" for Custom tests, so they don't interfere with grading
	//RunTestSet( false, TestSet_Custom_Dummy,	 "Custom dummy sample tests" );
	RunTestSet( false, TestSet_Custom_Rgba,		 "Custom Rgba sample tests" );
	RunTestSet( false, TestSet_Custom_MathUtils, "Custom MathUtils sample tests" );
	RunTestSet( false, TestSet_Custom_SetFromText, "Custom SetFromText sample tests" );
}


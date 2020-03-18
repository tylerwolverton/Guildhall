#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
AABB3::AABB3()
	: mins( Vec3( 0.f, 0.f, 0.f ) )
	, maxs( Vec3( 0.f, 0.f, 0.f ) )
{

}


//-----------------------------------------------------------------------------------------------
AABB3::AABB3( const Vec3& mins, const Vec3& maxs )
	: mins( mins )
	, maxs( maxs )
{

}


//-----------------------------------------------------------------------------------------------
AABB3::AABB3( float xMin, float yMin, float zMin, float xMax, float yMax, float zMax )
	: mins( Vec3( xMin, yMin, zMin ) )
	, maxs( Vec3( xMax, yMax, zMax ) )
{

}


//-----------------------------------------------------------------------------------------------
AABB3::~AABB3()
{
}


//-----------------------------------------------------------------------------------------------
void AABB3::SetFromText( const char* asText )
{
	std::vector<std::string> splitStrings = SplitStringOnDelimiter( asText, ',' );
	GUARANTEE_OR_DIE( splitStrings.size() == 6, Stringf( "AABB3 can't construct from improper string \"%s\"", asText ) );
	mins.x = (float)atof( splitStrings[0].c_str() );
	mins.y = (float)atof( splitStrings[1].c_str() );
	mins.z = (float)atof( splitStrings[2].c_str() );
	maxs.x = (float)atof( splitStrings[3].c_str() );
	maxs.y = (float)atof( splitStrings[4].c_str() );
	maxs.z = (float)atof( splitStrings[5].c_str() );
}


//-----------------------------------------------------------------------------------------------
void AABB3::Translate( const Vec3& translation )
{
	mins += translation;
	maxs += translation;
}


//-----------------------------------------------------------------------------------------------
Vec3 AABB3::GetCenter() const
{
	Vec3 diagonal = maxs - mins;
	return mins + ( diagonal * .5f );
}


//-----------------------------------------------------------------------------------------------
void AABB3::SetCenter( const Vec3& point )
{
	Vec3 halfDiagonal = ( maxs - mins ) * .5f;

	mins = point - halfDiagonal;
	maxs = point + halfDiagonal;
}

//-----------------------------------------------------------------------------------------------
Vec3 AABB3::GetDimensions() const
{
	return Vec3( maxs.x - mins.x, maxs.y - mins.y, maxs.z - mins.z );
}


//-----------------------------------------------------------------------------------------------
void AABB3::SetDimensions( const Vec3& dimensions )
{
	Vec3 center( GetCenter() );
	mins = center - ( dimensions * .5f );
	maxs = center + ( dimensions * .5f );
}

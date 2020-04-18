#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
// Static member definitions
const AABB2 AABB2::ONE_BY_ONE( Vec2( -.5f, -.5f ), Vec2( .5, .5 ) );
const AABB2 AABB2::ZERO_TO_ONE( Vec2::ZERO, Vec2::ONE );


//-----------------------------------------------------------------------------------------------
AABB2::AABB2()
	: mins( Vec2( 0.f, 0.f ) )
	, maxs( Vec2( 0.f, 0.f ) )
{
}


//-----------------------------------------------------------------------------------------------
AABB2::AABB2( const Vec2& mins, const Vec2& maxs )
	: mins( mins )
	, maxs( maxs )
{
}


//-----------------------------------------------------------------------------------------------
AABB2::AABB2( float xMin, float yMin, float xMax, float yMax )
	: mins( Vec2( xMin, yMin ) )
	, maxs( Vec2( xMax, yMax ) )
{
}


//-----------------------------------------------------------------------------------------------
AABB2::AABB2( const IntVec2& mins, const IntVec2& maxs )
	: mins( (Vec2)mins )
	, maxs( (Vec2)maxs )
{
}


//-----------------------------------------------------------------------------------------------
AABB2::AABB2( int xMin, int yMin, int xMax, int yMax )
	: mins( Vec2( (float)xMin, (float)yMin ) )
	, maxs( Vec2( (float)xMax, (float)yMax ) )
{
}


//-----------------------------------------------------------------------------------------------
void AABB2::SetFromText( const char* asText )
{
	std::vector<std::string> splitStrings = SplitStringOnDelimiter( asText, ',' );
	GUARANTEE_OR_DIE( splitStrings.size() == 4, Stringf( "AABB2 can't construct from improper string \"%s\"", asText ) );
	mins.x = (float)atof( splitStrings[0].c_str() );
	mins.y = (float)atof( splitStrings[1].c_str() );
	maxs.x = (float)atof( splitStrings[2].c_str() );
	maxs.y = (float)atof( splitStrings[3].c_str() );
}


//-----------------------------------------------------------------------------------------------
AABB2::~AABB2()
{
}


//-----------------------------------------------------------------------------------------------
void AABB2::Translate( const Vec2& translation )
{
	mins += translation;
	maxs += translation;
}


//-----------------------------------------------------------------------------------------------
bool AABB2::IsPointInside( const Vec2& point ) const
{
	return ( point.x > mins.x
			 && point.x < maxs.x
			 && point.y > mins.y
			 && point.y < maxs.y );
}


//-----------------------------------------------------------------------------------------------
void AABB2::StretchToIncludePoint( const Vec2& newPoint )
{
	if ( newPoint.x < mins.x )
	{
		mins.x = newPoint.x;
	}
	if ( newPoint.x > maxs.x )
	{
		maxs.x = newPoint.x;
	}
	if ( newPoint.y < mins.y )
	{
		mins.y = newPoint.y;
	}
	if ( newPoint.y > maxs.y )
	{
		maxs.y = newPoint.y;
	}
}


//-----------------------------------------------------------------------------------------------
void AABB2::StretchToIncludePointMaintainAspect( const Vec2& newPoint, const Vec2& aspectDimensions )
{
	float xRatio = aspectDimensions.x / aspectDimensions.y;
	float yRatio = aspectDimensions.y / aspectDimensions.x;

	if ( newPoint.x < mins.x )
	{
		mins.y -= ( mins.x - newPoint.x ) * yRatio;
		mins.x = newPoint.x;
	}
	if ( newPoint.x > maxs.x )
	{
		maxs.y += ( newPoint.x - maxs.x ) * yRatio;
		maxs.x = newPoint.x;
	}
	if ( newPoint.y < mins.y )
	{
		mins.x -= ( mins.y - newPoint.y ) * xRatio;
		mins.y = newPoint.y;
	}
	if ( newPoint.y > maxs.y )
	{
		maxs.x += ( newPoint.y - maxs.y ) * xRatio;
		maxs.y = newPoint.y;
	}
}


//-----------------------------------------------------------------------------------------------
bool AABB2::OverlapsWith( const AABB2& otherBox ) const
{
	return DoAABBsOverlap2D( *this, otherBox );
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetBoxWithin( const Vec2& dimensions, const Vec2& alignment ) const
{
	float width = maxs.x - mins.x;
	float height = maxs.y - mins.y;

	Vec2 innerMins( alignment.x * width, alignment.y * height );
	Vec2 innerMaxs( mins + dimensions );

	return AABB2( innerMins, innerMaxs );
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetBoxAtLeft( float fractionOfWidth, float additionalWidth ) const
{
	float width = maxs.x - mins.x;
	float innerBoxWidth = ( width * fractionOfWidth ) + additionalWidth;

	if ( innerBoxWidth > width )
	{
		innerBoxWidth = width;
	}
	
	AABB2 innerBox( mins.x, mins.y, mins.x + innerBoxWidth, maxs.y );
	return innerBox;
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetBoxAtRight( float fractionOfWidth, float additionalWidth ) const
{
	float width = maxs.x - mins.x;
	float innerBoxWidth = ( width * fractionOfWidth ) + additionalWidth;

	if ( innerBoxWidth > width )
	{
		innerBoxWidth = width;
	}
	
	AABB2 innerBox( maxs.x - innerBoxWidth, mins.y, maxs.x, maxs.y );
	return innerBox;
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetBoxAtTop( float fractionOfHeight, float additionalHeight ) const
{
	float height = maxs.y - mins.y;
	float innerBoxHeight = ( height * fractionOfHeight ) + additionalHeight;

	if ( innerBoxHeight > height )
	{
		innerBoxHeight = height;
	}

	AABB2 innerBox( mins.x, maxs.y - innerBoxHeight, maxs.x, maxs.y );
	return innerBox;
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetBoxAtBottom( float fractionOfHeight, float additionalHeight ) const
{
	float height = maxs.y - mins.y;
	float innerBoxHeight = ( height * fractionOfHeight ) + additionalHeight;

	if ( innerBoxHeight > height )
	{
		innerBoxHeight = height;
	}

	AABB2 innerBox( mins.x, mins.y, maxs.x, mins.y + innerBoxHeight );
	return innerBox;
}


//-----------------------------------------------------------------------------------------------
float AABB2::GetWidth() const
{
	return maxs.x - mins.x;
}


//-----------------------------------------------------------------------------------------------
float AABB2::GetHeight() const
{
	return maxs.y - mins.y;
}


//-----------------------------------------------------------------------------------------------
float AABB2::GetOuterRadius() const
{
	float width = maxs.x - mins.x;
	float height = maxs.y - mins.y;
	
	return ( width * .5f ) + ( height * .5f );
}


//-----------------------------------------------------------------------------------------------
float AABB2::GetInnerRadius() const
{
	float width = maxs.x - mins.x; 
	float height = maxs.y - mins.y;

	if ( width < height )
	{
		return width * .5f;
	}
	else
	{
		return height * .5f;
	}
}


//-----------------------------------------------------------------------------------------------
void AABB2::GetCornerPositions( Vec2* out_fourPoints ) const
{
	out_fourPoints[0] = mins;
	out_fourPoints[1] = Vec2( maxs.x, mins.y );
	out_fourPoints[2] = Vec2( mins.x, maxs.y );
	out_fourPoints[3] = maxs;
}


//-----------------------------------------------------------------------------------------------
void AABB2::GetCornerPositionsCCW( Vec2* out_fourPoints ) const
{
	out_fourPoints[0] = mins;
	out_fourPoints[1] = Vec2( maxs.x, mins.y );
	out_fourPoints[2] = maxs;
	out_fourPoints[3] = Vec2( mins.x, maxs.y );
}


//-----------------------------------------------------------------------------------------------
void AABB2::GetPositionOnEachEdge( float wallFraction, Vec2* out_fourPoints ) const
{
	// TODO: Roll rng for each point on wall using FloatRange

	// Move corner positions over based on fraction to get random point on edge
	Vec2 corners[4];
	GetCornerPositions( corners );

	out_fourPoints[0] = InterpolatePoint2D( corners[0], corners[1], wallFraction );
	out_fourPoints[1] = InterpolatePoint2D( corners[0], corners[2], wallFraction );
	out_fourPoints[2] = InterpolatePoint2D( corners[1], corners[3], wallFraction );
	out_fourPoints[3] = InterpolatePoint2D( corners[2], corners[3], wallFraction );
}


//-----------------------------------------------------------------------------------------------
const Vec2 AABB2::GetPointAtUV( const Vec2& uv ) const
{
	return Vec2( Interpolate( mins.x, maxs.x, uv.x ),
				 Interpolate( mins.y, maxs.y, uv.y ) );
}


//-----------------------------------------------------------------------------------------------
Vec2 AABB2::GetUVForPoint( Vec2 point ) const
{
	return Vec2( RangeMapFloat( mins.x, maxs.x, 0.f, 1.f, point.x ),
				 RangeMapFloat( mins.y, maxs.y, 0.f, 1.f, point.y ) );
}


//-----------------------------------------------------------------------------------------------
Vec2 AABB2::GetCenter() const
{
	Vec2 diagonal = maxs - mins;
	return mins + ( diagonal * .5f );
}


//-----------------------------------------------------------------------------------------------
Vec2 AABB2::GetDimensions() const
{
	return Vec2( maxs.x - mins.x, maxs.y - mins.y );
}


//-----------------------------------------------------------------------------------------------
Vec2 AABB2::GetNearestPoint( const Vec2& point ) const
{
	if ( IsPointInside( point ) )
	{
		return point;
	}

	// Point is to the right of box
	if ( point.x > maxs.x )
	{
		// Check if either of the corners is the closest point
		if ( point.y > maxs.y )
		{
			return maxs;
		}
		else if ( point.y < mins.y )
		{
			return Vec2( maxs.x, mins.y );
		}

		return Vec2( maxs.x, point.y );
	}
	// Point is to the left of box
	else if ( point.x < mins.x )
	{
		// Check if either of the corners is the closest point
		if ( point.y > maxs.y )
		{
			return Vec2( mins.x, maxs.y );
		}
		else if ( point.y < mins.y )
		{
			return mins;
		}

		return Vec2( mins.x, point.y );
	}

	// Check if point is above or below the box but still inside the x range
	if ( point.y > maxs.y )
	{
		return Vec2( point.x, maxs.y );
	}
	else if ( point.y < maxs.y )
	{
		return Vec2( point.x, mins.y );
	}

	// This can never be reached
	return Vec2();
}


//-----------------------------------------------------------------------------------------------
void AABB2::SetCenter( const Vec2& point )
{
	Vec2 halfDiagonal = ( maxs - mins ) * .5f;

	mins = point - halfDiagonal;
	maxs = point + halfDiagonal;
}


//-----------------------------------------------------------------------------------------------
void AABB2::SetDimensions( const Vec2& dimensions )
{
	Vec2 center( GetCenter() );
	mins = center - ( dimensions * .5f );
	maxs = center + ( dimensions * .5f );
}


//-----------------------------------------------------------------------------------------------
void AABB2::FitWithinBounds( const AABB2& boundingBox )
{
	if ( mins.x < boundingBox.mins.x )
	{
		Translate( Vec2( boundingBox.mins.x - mins.x, 0.f ) );
	}
	if ( maxs.x > boundingBox.maxs.x )
	{
		Translate( Vec2( boundingBox.maxs.x - maxs.x, 0.f ) );
	}

	if ( mins.y < boundingBox.mins.y )
	{
		Translate( Vec2( 0.f, boundingBox.mins.y - mins.y ) );
	}
	if ( maxs.y > boundingBox.maxs.y )
	{
		Translate( Vec2( 0.f, boundingBox.maxs.y - maxs.y ) );
	}
}


//-----------------------------------------------------------------------------------------------
void AABB2::ChopOffLeft( float fractionOfWidth, float additionalWidth )
{
	float width = maxs.x - mins.x;
	float chopWidth = ( width * fractionOfWidth ) + additionalWidth;

	if ( chopWidth > width )
	{
		chopWidth = width;
	}

	mins.x = mins.x + chopWidth;
}


//-----------------------------------------------------------------------------------------------
void AABB2::ChopOffRight( float fractionOfWidth, float additionalWidth )
{
	float width = maxs.x - mins.x;
	float chopWidth = ( width * fractionOfWidth ) + additionalWidth;

	if ( chopWidth > width )
	{
		chopWidth = width;
	}

	maxs.x = maxs.x - chopWidth;
}


//-----------------------------------------------------------------------------------------------
void AABB2::ChopOffTop( float fractionOfHeight, float additionalHeight )
{
	float height = maxs.y - mins.y;
	float chopHeight = ( height * fractionOfHeight ) + additionalHeight;

	if ( chopHeight > height )
	{
		chopHeight = height;
	}

	maxs.y = maxs.y - chopHeight;
}


//-----------------------------------------------------------------------------------------------
void AABB2::ChopOffBottom( float fractionOfHeight, float additionalHeight )
{
	float height = maxs.y - mins.y;
	float chopHeight = ( height * fractionOfHeight ) + additionalHeight;

	if ( chopHeight > height )
	{
		chopHeight = height;
	}

	mins.y = mins.y + chopHeight;
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetChoppedOffLeft( float fractionOfWidth, float additionalWidth )
{
	float width = maxs.x - mins.x;
	float chopWidth = ( width * fractionOfWidth ) + additionalWidth;

	if ( chopWidth > width )
	{
		chopWidth = width;
	}

	float splitX = mins.x + chopWidth;
	
	AABB2 choppedBox( mins.x, mins.y, splitX, maxs.y );

	mins.x = splitX;

	return choppedBox;
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetChoppedOffRight( float fractionOfWidth, float additionalWidth )
{
	float width = maxs.x - mins.x;
	float chopWidth = ( width * fractionOfWidth ) + additionalWidth;

	if ( chopWidth > width )
	{
		chopWidth = width;
	}

	float splitX = maxs.x - chopWidth;

	AABB2 choppedBox( splitX, mins.y, maxs.x, maxs.y );

	maxs.x = splitX;

	return choppedBox;
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetChoppedOffTop( float fractionOfHeight, float additionalHeight )
{
	float height = maxs.y - mins.y;
	float chopHeight = ( height * fractionOfHeight ) + additionalHeight;

	if ( chopHeight > height )
	{
		chopHeight = height;
	}

	float splitY = maxs.y - chopHeight;
	
	AABB2 choppedBox( mins.x, splitY, maxs.x, maxs.y );
	
	maxs.y = splitY;

	return choppedBox;
}


//-----------------------------------------------------------------------------------------------
const AABB2 AABB2::GetChoppedOffBottom( float fractionOfHeight, float additionalHeight )
{
	float height = maxs.y - mins.y;
	float chopHeight = ( height * fractionOfHeight ) + additionalHeight;

	if ( chopHeight > height )
	{
		chopHeight = height;
	}

	float splitY = mins.y + chopHeight;

	AABB2 choppedBox( mins.x, mins.y, maxs.x, splitY );

	mins.y = splitY;

	return choppedBox;
}

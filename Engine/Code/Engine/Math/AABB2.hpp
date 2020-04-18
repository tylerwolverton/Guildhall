#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
struct IntVec2;


//-----------------------------------------------------------------------------------------------
struct AABB2
{
public:
	Vec2 mins;
	Vec2 maxs;

	static const AABB2 ONE_BY_ONE;
	static const AABB2 ZERO_TO_ONE;

public:
	AABB2();
	~AABB2();
	explicit AABB2( const Vec2& mins, const Vec2& maxs );
	explicit AABB2( float xMin, float yMin, float xMax, float yMax );
	explicit AABB2( const IntVec2& mins, const IntVec2& maxs );     // Should these go into an IntAABB2 instead (they help with snapping sub boxes of the map)?
	explicit AABB2( int xMin, int yMin, int xMax, int yMax );
	void SetFromText( const char* asText );

	// Transforms
	void		Translate( const Vec2& translation );

	// UVs
	const Vec2	GetPointAtUV( const Vec2& uv ) const;
	Vec2		GetUVForPoint( Vec2 point ) const;

	// Size and Center
	Vec2		GetCenter() const;
	void		SetCenter( const Vec2& point );
	Vec2		GetDimensions() const;
	void		SetDimensions( const Vec2& dimensions );
	void		FitWithinBounds( const AABB2& boundingBox );

	// Geometric Queries
	bool		IsPointInside						( const Vec2& point ) const;
	Vec2		GetNearestPoint						( const	Vec2& point ) const;
	void		StretchToIncludePoint				( const Vec2& newPoint );
	void		StretchToIncludePointMaintainAspect	( const Vec2& newPoint, const Vec2& aspectDimensions );
	bool		OverlapsWith						( const AABB2& otherBox ) const;

	// Accessors 
	const AABB2 GetBoxWithin( const Vec2& dimensions, const Vec2& alignment ) const;
	const AABB2 GetBoxAtLeft( float fractionOfWidth, float additionalWidth = 0.f ) const;
	const AABB2 GetBoxAtRight( float fractionOfWidth, float additionalWidth = 0.f ) const;
	const AABB2 GetBoxAtTop( float fractionOfHeight, float additionalHeight = 0.f ) const;
	const AABB2 GetBoxAtBottom( float fractionOfHeight, float additionalHeight = 0.f ) const;
	float		GetWidth() const;
	float		GetHeight() const;
	float		GetOuterRadius() const;
	float		GetInnerRadius() const;
	void		GetCornerPositions( Vec2* out_fourPoints ) const;								// fill float[4] with corners
	void		GetCornerPositionsCCW( Vec2* out_fourPoints ) const;								// fill float[4] with corners
	void		GetPositionOnEachEdge( float wallFraction, Vec2* out_fourPoints ) const; // fill float[4] with points on edges

	// Chop Box
	void		ChopOffLeft( float fractionOfWidth, float additionalWidth );
	void		ChopOffRight( float fractionOfWidth, float additionalWidth );
	void		ChopOffTop( float fractionOfHeight, float additionalHeight );
	void		ChopOffBottom( float fractionOfHeight, float additionalHeight );
	const AABB2	GetChoppedOffLeft( float fractionOfWidth, float additionalWidth = 0.f );
	const AABB2	GetChoppedOffRight( float fractionOfWidth, float additionalWidth = 0.f );
	const AABB2	GetChoppedOffTop( float fractionOfHeight, float additionalHeight = 0.f );
	const AABB2	GetChoppedOffBottom( float fractionOfHeight, float additionalHeight = 0.f );
};

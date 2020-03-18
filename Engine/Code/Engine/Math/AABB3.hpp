#pragma once
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
struct AABB3
{
public:
	Vec3 mins;
	Vec3 maxs;
	
public:
	AABB3();
	~AABB3();
	explicit AABB3( const Vec3& mins, const Vec3& maxs );
	explicit AABB3( float xMin, float yMin, float zMin, float xMax, float yMax, float zMax );
	void SetFromText( const char* asText );

	// Transforms
	void		Translate( const Vec3& translation );
	
	// Size and Center
	Vec3		GetCenter() const;
	void		SetCenter( const Vec3& point );
	Vec3		GetDimensions() const;
	void		SetDimensions( const Vec3& dimensions );
};

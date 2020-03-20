#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
struct OBB2
{
public:
	Vec2 m_center = Vec2::ZERO;
	Vec2 m_halfDimensions = Vec2::ZERO;
	Vec2 m_iBasis = Vec2( 1.f, 0.f );

public:
	OBB2() = default;
	~OBB2() = default;
	OBB2( const OBB2& copyFrom );
	explicit OBB2( const Vec2& center, const Vec2& fullDimensions, const Vec2& iBasisNormal = Vec2( 1.f, 0.f ) );
	explicit OBB2( const Vec2& center, const Vec2& fullDimensions, float orientationDegrees );
	explicit OBB2( const AABB2& asAxisAlignedBox, float orientationDegrees = 0.f );

	// Accessors (const methods)
	bool		IsPointInside( const Vec2& point ) const; // does NOT include points on edge
	const Vec2	GetCenter() const																			{ return m_center; }
	const Vec2	GetDimensions() const																		{ return m_halfDimensions * 2.f; }
	const Vec2	GetHalfDimensions() const																	{ return m_halfDimensions; }
	const Vec2	GetIBasisNormal() const																		{ return m_iBasis; }
	const Vec2	GetJBasisNormal() const;
	float		GetOrientationDegrees() const;
	const Vec2	GetNearestPoint( const Vec2& point ) const;
	const Vec2	GetPointAtUV( const Vec2& uvCoordsZeroToOne ) const;
	const Vec2	GetUVForPoint( const Vec2& point ) const;
	float		GetOuterRadius() const;									// radius of circle entirely containing the OBB (= w/2 + h/2)
	float		GetInnerRadius() const;									// radius of circle entirely within the OBB (tightly)
	void		GetCornerPositions( Vec2* out_fourPoints ) const;		// fill float[4] with corners

	// Mutators (non-const methods)
	void		Translate( const Vec2& translation );
	void		SetCenter( const Vec2& newCenter );
	void		SetDimensions( const Vec2& newDimensions );
	void		SetOrientationDegrees( float newAbsoluteOrientation );
	void		RotateByDegrees( float relativeRotationDegrees );
	void		StretchToIncludePoint( const Vec2& point );
	void		Fix();	// "corrects" the box: abs() halfDimensions and normalize iBasis (or restore to (1,0) if at (0,0))

	// Operators (self-mutating / non-const)
	void operator=( const OBB2& assignFrom );

private:
	const Vec2 GetPointInLocalSpaceFromWorldSpace( const Vec2& point );
};

#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"


//-----------------------------------------------------------------------------------------------
struct OBB3
{
public:
	Vec3 m_center = Vec3::ZERO;
	Vec3 m_halfDimensions = Vec3::ZERO;
	Vec3 m_iBasis = Vec3( 1.f, 0.f, 0.f );
	Vec3 m_jBasis = Vec3( 0.f, 1.f, 0.f );

public:
	OBB3() = default;
	~OBB3() = default;
	OBB3( const OBB3& copyFrom );
	explicit OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& iBasisNormal = Vec3( 1.f, 0.f, 0.f ), const Vec3& jBasisNormal = Vec3( 0.f, 1.f, 0.f ) );

	// Accessors (const methods)
	const Vec3	GetCenter() const							{ return m_center; }
	const Vec3	GetDimensions() const						{ return m_halfDimensions * 2.f; }
	const Vec3	GetHalfDimensions() const					{ return m_halfDimensions; }
	const Vec3	GetIBasisNormal() const						{ return m_iBasis; }
	const Vec3	GetJBasisNormal() const						{ return m_jBasis; }
	const Vec3	GetKBasisNormal() const;
	void		GetCornerPositions( Vec3* out_eightPoints ) const;		// fill float[8] with corners

	// Mutators (non-const methods)
	void		Translate( const Vec3& translation );
	void		SetCenter( const Vec3& newCenter );
	void		SetDimensions( const Vec3& newDimensions );
	/*void		SetOrientationDegrees( float newAbsoluteOrientation );
	void		RotateByDegrees( float relativeRotationDegrees );*/
	void		Fix();	// "corrects" the box: abs() halfDimensions and normalize bases ( or restore to default if at (0,0) )

	// Operators (self-mutating / non-const)
	void operator=( const OBB3& assignFrom );

private:
	//const Vec2 GetPointInLocalSpaceFromWorldSpace( const Vec3& point );
};

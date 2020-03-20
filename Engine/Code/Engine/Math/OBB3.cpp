#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <math.h>


//-----------------------------------------------------------------------------------------------
OBB3::OBB3( const OBB3& copyFrom )
	: m_center( copyFrom.m_center )
	, m_halfDimensions( copyFrom.m_halfDimensions )
	, m_iBasis( copyFrom.m_iBasis )
	, m_jBasis( copyFrom.m_jBasis )
{
}


//-----------------------------------------------------------------------------------------------
OBB3::OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& iBasisNormal, const Vec3& jBasisNormal )
	: m_center( center )
	, m_halfDimensions( fullDimensions * .5f )
	, m_iBasis( iBasisNormal )
	, m_jBasis( jBasisNormal )
{
}


//-----------------------------------------------------------------------------------------------
OBB3::OBB3( const AABB3& asAxisAlignedBox, const Vec3& orientationDegrees )
{

}


//-----------------------------------------------------------------------------------------------
OBB3::OBB3( const Vec3& center, const Vec3& fullDimensions, float pitch, float yaw, float roll )
	: m_center( center )
	, m_halfDimensions( fullDimensions * .5f )
{
	Transform trans;
	trans.SetRotationFromPitchRollYawDegrees( pitch, roll, yaw );

	Mat44 rotation = trans.GetAsMatrix();

	rotation.TransformVector3D( m_iBasis );
	rotation.TransformVector3D( m_jBasis );
}


//-----------------------------------------------------------------------------------------------
const Vec3 OBB3::GetKBasisNormal() const
{
	Vec3 kBasis = CrossProduct3D( m_iBasis, m_jBasis );
	return kBasis.GetNormalized();
}


//-----------------------------------------------------------------------------------------------
void OBB3::GetCornerPositions( Vec3* out_eightPoints ) const
{
	Vec3 boxHalfWidth(	m_halfDimensions.x * m_iBasis );
	Vec3 boxHalfHeight( m_halfDimensions.y * m_jBasis );
	Vec3 boxHalfDepth(	m_halfDimensions.z * GetKBasisNormal() );

	Vec3 topFrontRight(		m_center + boxHalfWidth + boxHalfHeight - boxHalfDepth );
	Vec3 topFrontLeft(		m_center - boxHalfWidth + boxHalfHeight - boxHalfDepth );
	Vec3 bottomFrontLeft(	m_center - boxHalfWidth - boxHalfHeight - boxHalfDepth );
	Vec3 bottomFrontRight(	m_center + boxHalfWidth - boxHalfHeight - boxHalfDepth );

	Vec3 topBackRight(		m_center + boxHalfWidth + boxHalfHeight + boxHalfDepth );
	Vec3 topBackLeft(		m_center - boxHalfWidth + boxHalfHeight + boxHalfDepth );
	Vec3 bottomBackLeft(	m_center - boxHalfWidth - boxHalfHeight + boxHalfDepth );
	Vec3 bottomBackRight(	m_center + boxHalfWidth - boxHalfHeight + boxHalfDepth );

	out_eightPoints[0] = bottomFrontLeft;
	out_eightPoints[1] = bottomFrontRight;
	out_eightPoints[2] = topFrontLeft;
	out_eightPoints[3] = topFrontRight;

	out_eightPoints[4] = bottomBackLeft;
	out_eightPoints[5] = bottomBackRight;
	out_eightPoints[6] = topBackLeft;
	out_eightPoints[7] = topBackRight;
}


//-----------------------------------------------------------------------------------------------
void OBB3::Translate( const Vec3& translation )
{
	m_center += translation;
}


//-----------------------------------------------------------------------------------------------
void OBB3::SetCenter( const Vec3& newCenter )
{
	m_center = newCenter;
}


//-----------------------------------------------------------------------------------------------
void OBB3::SetDimensions( const Vec3& newDimensions )
{
	m_halfDimensions = newDimensions * .5f;
}


//-----------------------------------------------------------------------------------------------
void OBB3::SetOrientationDegrees( float newAbsoluteOrientation )
{

}


//-----------------------------------------------------------------------------------------------
void OBB3::RotateByDegrees( float relativeRotationDegrees )
{

}


//-----------------------------------------------------------------------------------------------
void OBB3::Fix()
{
	// Ensure dimensions aren't negative
	m_halfDimensions.x = fabsf( m_halfDimensions.x );
	m_halfDimensions.y = fabsf( m_halfDimensions.y );
	m_halfDimensions.z = fabsf( m_halfDimensions.z );

	// Normalize i basis
	if ( m_iBasis == Vec3::ZERO )
	{
		m_iBasis = Vec3( 1.f, 0.f, 0.f );
	}
	else
	{
		m_iBasis.Normalize();
	}

	// Normalize j basis
	if ( m_jBasis == Vec3::ZERO )
	{
		m_jBasis = Vec3( 0.f, 1.f, 0.f );
	}
	else
	{
		m_jBasis.Normalize();
	}
}


//-----------------------------------------------------------------------------------------------
//const Vec2 OBB3::GetPointInLocalSpaceFromWorldSpace( const Vec3& point )
//{
//
//}


//-----------------------------------------------------------------------------------------------
void OBB3::operator=( const OBB3& assignFrom )
{
	m_center = assignFrom.m_center;
	m_halfDimensions = assignFrom.m_halfDimensions;
	m_iBasis = assignFrom.m_iBasis;
	m_jBasis = assignFrom.m_jBasis;
}

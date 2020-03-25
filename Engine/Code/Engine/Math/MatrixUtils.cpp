#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Mat44.hpp"


//-----------------------------------------------------------------------------------------------
Mat44 MakeOrthographicProjectionMatrixD3D( float minX, float maxX, 
										   float minY, float maxY, 
										   float minZ, float maxZ )
{
	Vec3 mins( minX, minY, minZ );
	Vec3 maxs( maxX, maxY, maxZ );
	return Mat44::CreateOrthographicProjection( mins, maxs );
}


//-----------------------------------------------------------------------------------------------
Mat44 MakePerspectiveProjectionMatrixD3D( float fovDegrees, 
										  float aspectRatio, 
										  float nearZ, float farZ )
{
	return Mat44::CreatePerspectiveProjection( fovDegrees, aspectRatio, nearZ, farZ );
}


//-----------------------------------------------------------------------------------------------
Mat44 MakeLookAtMatrix( const Vec3& pos, const Vec3& target, const Vec3& worldUp )
{
	Vec3 forward = target - pos;
	forward.Normalize();
	
	Vec3 right = CrossProduct3D( forward, worldUp );
	right.Normalize();

	Vec3 up = CrossProduct3D( right, forward );

	Mat44 lookAt;
	lookAt.SetBasisVectors3D( right, up, forward );

	return lookAt;
}


//-----------------------------------------------------------------------------------------------
void TransposeMatrix( Mat44& matrix )
{
	matrix.Transpose();
}


//-----------------------------------------------------------------------------------------------
bool IsOrthoNormalMatrix( Mat44& matrix )
{
	Vec3 iBasis = matrix.GetIBasis3D();
	Vec3 jBasis = matrix.GetJBasis3D();
	Vec3 kBasis = matrix.GetKBasis3D();

	if ( !IsNearlyEqual( iBasis.GetLength(), 1.f )
		 || !IsNearlyEqual( jBasis.GetLength(), 1.f )
		 || !IsNearlyEqual( kBasis.GetLength(), 1.f ) )
	{
		return false;
	}

	if ( !IsNearlyEqual( DotProduct3D( iBasis, jBasis ), 0.f )
		 || !IsNearlyEqual( DotProduct3D( jBasis, kBasis ), 0.f )
		 || !IsNearlyEqual( DotProduct3D( iBasis, kBasis ), 0.f ) )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
void InvertOrthoNormalMatrix( Mat44& matrix )
{
	Mat44 inverseMatrix = matrix;
	inverseMatrix.SetTranslation3D( Vec3::ZERO );
	inverseMatrix.Transpose();

	Vec3 translation = matrix.GetTranslation3D();
	Vec3 inverseTranslation = inverseMatrix.TransformPosition3D( -translation );
	inverseMatrix.SetTranslation3D( inverseTranslation );

	matrix = inverseMatrix;
}


//-----------------------------------------------------------------------------------------------
void InvertMatrix( Mat44& matrix )
{
	if ( IsOrthoNormalMatrix( matrix ) )
	{
		InvertOrthoNormalMatrix( matrix );
		return;
	}

	double inv[16];
	double det;
	double m[16];
	uint i;

	for ( i = 0; i < 16; ++i ) 
	{
		m[i] = (double)matrix.GetAsFloatArray()[i];
	}

	inv[0] =	m[5] * m[10] * m[15] -
				m[5] * m[11] * m[14] -
				m[9] * m[6] * m[15] +
				m[9] * m[7] * m[14] +
				m[13] * m[6] * m[11] -
				m[13] * m[7] * m[10];

	inv[4] =	-m[4] * m[10] * m[15] +
				m[4] * m[11] * m[14] +
				m[8] * m[6] * m[15] -
				m[8] * m[7] * m[14] -
				m[12] * m[6] * m[11] +
				m[12] * m[7] * m[10];

	inv[8] =	m[4] * m[9] * m[15] -
				m[4] * m[11] * m[13] -
				m[8] * m[5] * m[15] +
				m[8] * m[7] * m[13] +
				m[12] * m[5] * m[11] -
				m[12] * m[7] * m[9];

	inv[12] =	-m[4] * m[9] * m[14] +
				m[4] * m[10] * m[13] +
				m[8] * m[5] * m[14] -
				m[8] * m[6] * m[13] -
				m[12] * m[5] * m[10] +
				m[12] * m[6] * m[9];

	inv[1] =	-m[1] * m[10] * m[15] +
				m[1] * m[11] * m[14] +
				m[9] * m[2] * m[15] -
				m[9] * m[3] * m[14] -
				m[13] * m[2] * m[11] +
				m[13] * m[3] * m[10];

	inv[5] =	m[0] * m[10] * m[15] -
				m[0] * m[11] * m[14] -
				m[8] * m[2] * m[15] +
				m[8] * m[3] * m[14] +
				m[12] * m[2] * m[11] -
				m[12] * m[3] * m[10];

	inv[9] =	-m[0] * m[9] * m[15] +
				m[0] * m[11] * m[13] +
				m[8] * m[1] * m[15] -
				m[8] * m[3] * m[13] -
				m[12] * m[1] * m[11] +
				m[12] * m[3] * m[9];

	inv[13] =	m[0] * m[9] * m[14] -
				m[0] * m[10] * m[13] -
				m[8] * m[1] * m[14] +
				m[8] * m[2] * m[13] +
				m[12] * m[1] * m[10] -
				m[12] * m[2] * m[9];

	inv[2] =	m[1] * m[6] * m[15] -
				m[1] * m[7] * m[14] -
				m[5] * m[2] * m[15] +
				m[5] * m[3] * m[14] +
				m[13] * m[2] * m[7] -
				m[13] * m[3] * m[6];

	inv[6] =	-m[0] * m[6] * m[15] +
				m[0] * m[7] * m[14] +
				m[4] * m[2] * m[15] -
				m[4] * m[3] * m[14] -
				m[12] * m[2] * m[7] +
				m[12] * m[3] * m[6];

	inv[10] =	m[0] * m[5] * m[15] -
				m[0] * m[7] * m[13] -
				m[4] * m[1] * m[15] +
				m[4] * m[3] * m[13] +
				m[12] * m[1] * m[7] -
				m[12] * m[3] * m[5];

	inv[14] =	-m[0] * m[5] * m[14] +
				m[0] * m[6] * m[13] +
				m[4] * m[1] * m[14] -
				m[4] * m[2] * m[13] -
				m[12] * m[1] * m[6] +
				m[12] * m[2] * m[5];

	inv[3] =	-m[1] * m[6] * m[11] +
				m[1] * m[7] * m[10] +
				m[5] * m[2] * m[11] -
				m[5] * m[3] * m[10] -
				m[9] * m[2] * m[7] +
				m[9] * m[3] * m[6];

	inv[7] =	m[0] * m[6] * m[11] -
				m[0] * m[7] * m[10] -
				m[4] * m[2] * m[11] +
				m[4] * m[3] * m[10] +
				m[8] * m[2] * m[7] -
				m[8] * m[3] * m[6];

	inv[11] =	-m[0] * m[5] * m[11] +
				m[0] * m[7] * m[9] +
				m[4] * m[1] * m[11] -
				m[4] * m[3] * m[9] -
				m[8] * m[1] * m[7] +
				m[8] * m[3] * m[5];

	inv[15] =	m[0] * m[5] * m[10] -
				m[0] * m[6] * m[9] -
				m[4] * m[1] * m[10] +
				m[4] * m[2] * m[9] +
				m[8] * m[1] * m[6] -
				m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	det = 1.0 / det;

	Mat44 ret;
	for ( i = 0; i < 16; i++ ) 
	{
		ret.GetAsFloatArray()[i] = (float)( inv[i] * det );
	}

	matrix = ret;
}

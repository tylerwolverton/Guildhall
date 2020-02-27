#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Mat44::Mat44( float* sixteenValuesBasisMajor )
{
	Ix = sixteenValuesBasisMajor[0];
	Iy = sixteenValuesBasisMajor[1];
	Iz = sixteenValuesBasisMajor[2];
	Iw = sixteenValuesBasisMajor[3];

	Jx = sixteenValuesBasisMajor[4];
	Jy = sixteenValuesBasisMajor[5];
	Jz = sixteenValuesBasisMajor[6];
	Jw = sixteenValuesBasisMajor[7];

	Kx = sixteenValuesBasisMajor[8];
	Ky = sixteenValuesBasisMajor[9];
	Kz = sixteenValuesBasisMajor[10];
	Kw = sixteenValuesBasisMajor[11];

	Tx = sixteenValuesBasisMajor[12];
	Ty = sixteenValuesBasisMajor[13];
	Tz = sixteenValuesBasisMajor[14];
	Tw = sixteenValuesBasisMajor[15];
}


//-----------------------------------------------------------------------------------------------
Mat44::Mat44( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D )
{
	Ix = iBasis2D.x;
	Iy = iBasis2D.y;

	Jx = jBasis2D.x;
	Jy = jBasis2D.y;

	Tx = translation2D.x;
	Ty = translation2D.y;
}


//-----------------------------------------------------------------------------------------------
Mat44::Mat44( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D )
{
	Ix = iBasis3D.x;
	Iy = iBasis3D.y;
	Iz = iBasis3D.z;

	Jx = jBasis3D.x;
	Jy = jBasis3D.y;
	Jz = jBasis3D.z;
	
	Kx = kBasis3D.x;
	Ky = kBasis3D.y;
	Kz = kBasis3D.z;

	Tx = translation3D.x;
	Ty = translation3D.y;
	Tz = translation3D.z;
}


//-----------------------------------------------------------------------------------------------
Mat44::Mat44( const Vec4& iBasisHomogeneous, const Vec4& jBasisHomogeneous, const Vec4& kBasisHomogeneous, const Vec4& translationHomogeneous )
{
	Ix = iBasisHomogeneous.x;
	Iy = iBasisHomogeneous.y;
	Iz = iBasisHomogeneous.z;
	Iw = iBasisHomogeneous.w;

	Jx = jBasisHomogeneous.x;
	Jy = jBasisHomogeneous.y;
	Jz = jBasisHomogeneous.z;
	Jw = jBasisHomogeneous.w;

	Kx = kBasisHomogeneous.x;
	Ky = kBasisHomogeneous.y;
	Kz = kBasisHomogeneous.z;
	Kw = kBasisHomogeneous.w;

	Tx = translationHomogeneous.x;
	Ty = translationHomogeneous.y;
	Tz = translationHomogeneous.z;
	Tw = translationHomogeneous.w;
}


//-----------------------------------------------------------------------------------------------
const Vec2 Mat44::TransformVector2D( const Vec2& vector ) const
{
	float newX = (Ix * vector.x ) + ( Jx * vector.y );
	float newY = (Iy * vector.x ) + ( Jy * vector.y );

	return Vec2( newX, newY );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Mat44::TransformVector3D( const Vec3& vector ) const
{
	float newX = ( Ix * vector.x ) + ( Jx * vector.y ) + ( Kx * vector.z );
	float newY = ( Iy * vector.x ) + ( Jy * vector.y ) + ( Ky * vector.z );
	float newZ = ( Iz * vector.x ) + ( Jz * vector.y ) + ( Kz * vector.z );

	return Vec3( newX, newY, newZ );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Mat44::TransformPosition2D( const Vec2& position ) const
{
	float newX = ( Ix * position.x ) + ( Jx * position.y ) + Tx;
	float newY = ( Iy * position.x ) + ( Jy * position.y ) + Ty;

	return Vec2( newX, newY );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Mat44::TransformPosition3D( const Vec3& position ) const
{
	float newX = ( Ix * position.x ) + ( Jx * position.y ) + ( Kx * position.z + Tx );
	float newY = ( Iy * position.x ) + ( Jy * position.y ) + ( Ky * position.z + Ty );
	float newZ = ( Iz * position.x ) + ( Jz * position.y ) + ( Kz * position.z + Tz );

	return Vec3( newX, newY, newZ );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Mat44::TransformHomogeneousPoint3D( const Vec4& vector ) const
{
	float newX = ( Ix * vector.x ) + ( Jx * vector.y ) + ( Kx * vector.z ) + ( vector.w * Tx );
	float newY = ( Iy * vector.x ) + ( Jy * vector.y ) + ( Ky * vector.z ) + ( vector.w * Ty );
	float newZ = ( Iz * vector.x ) + ( Jz * vector.y ) + ( Kz * vector.z ) + ( vector.w * Tz );
	float newW = ( Iw * vector.x ) + ( Jw * vector.y ) + ( Kw * vector.z ) + ( vector.w * Tw );

	return Vec4( newX, newY, newZ, newW );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Mat44::GetIBasis2D() const
{
	return Vec2( Ix, Iy );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Mat44::GetJBasis2D() const
{
	return Vec2( Jx, Jy );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Mat44::GetTranslation2D() const
{
	return Vec2( Tx, Ty );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Mat44::GetIBasis3D() const
{
	return Vec3( Ix, Iy, Iz );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Mat44::GetJBasis3D() const
{
	return Vec3( Jx, Jy, Jz );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Mat44::GetKBasis3D() const
{
	return Vec3( Kx, Ky, Kz );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Mat44::GetTranslation3D() const
{
	return Vec3( Tx, Ty, Tz );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Mat44::GetIBasis4D() const
{
	return Vec4( Ix, Iy, Iz, Iw );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Mat44::GetJBasis4D() const
{
	return Vec4( Jx, Jy, Jz, Jw );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Mat44::GetKBasis4D() const
{
	return Vec4( Kx, Ky, Kz, Kw );
}


//-----------------------------------------------------------------------------------------------
const Vec4 Mat44::GetTranslation4D() const
{
	return Vec4( Tx, Ty, Tz, Tw );
}


//-----------------------------------------------------------------------------------------------
void Mat44::SetTranslation2D( const Vec2& translation2D )
{
	Tx = translation2D.x;
	Ty = translation2D.y;
	Tz = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Mat44::SetTranslation3D( const Vec3& translation3D )
{
	Tx = translation3D.x;
	Ty = translation3D.y;
	Tz = translation3D.z;
}


//-----------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D )
{
	Ix = iBasis2D.x;
	Iy = iBasis2D.y;
	Iz = 0.f;
	Iw = 0.f;

	Jx = jBasis2D.x;
	Jy = jBasis2D.y;
	Jz = 0.f;
	Jw = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors2D( const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D )
{
	Ix = iBasis2D.x;
	Iy = iBasis2D.y;
	Iz = 0.f;
	Iw = 0.f;

	Jx = jBasis2D.x;
	Jy = jBasis2D.y;
	Jz = 0.f;
	Jw = 0.f;

	Tx = translation2D.x;
	Ty = translation2D.y;
	Tz = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D )
{
	Ix = iBasis3D.x;
	Iy = iBasis3D.y;
	Iz = iBasis3D.z;
	Iw = 0.f;

	Jx = jBasis3D.x;
	Jy = jBasis3D.y;
	Jz = jBasis3D.z;
	Jw = 0.f;

	Kx = kBasis3D.x;
	Ky = kBasis3D.y;
	Kz = kBasis3D.z;
	Kw = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors3D( const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D )
{
	Ix = iBasis3D.x;
	Iy = iBasis3D.y;
	Iz = iBasis3D.z;
	Iw = 0.f;

	Jx = jBasis3D.x;
	Jy = jBasis3D.y;
	Jz = jBasis3D.z;
	Jw = 0.f;

	Kx = kBasis3D.x;
	Ky = kBasis3D.y;
	Kz = kBasis3D.z;
	Kw = 0.f;

	Tx = translation3D.x;
	Ty = translation3D.y;
	Tz = translation3D.z;
}


//-----------------------------------------------------------------------------------------------
void Mat44::SetBasisVectors4D( const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& translation4D )
{
	Ix = iBasis4D.x;
	Iy = iBasis4D.y;
	Iz = iBasis4D.z;
	Iw = iBasis4D.w;

	Jx = jBasis4D.x;
	Jy = jBasis4D.y;
	Jz = jBasis4D.z;
	Jw = jBasis4D.w;

	Kx = kBasis4D.x;
	Ky = kBasis4D.y;
	Kz = kBasis4D.z;
	Kw = kBasis4D.w;

	Tx = translation4D.x;
	Ty = translation4D.y;
	Tz = translation4D.z;
	Tw = translation4D.w;
}


//-----------------------------------------------------------------------------------------------
void Mat44::RotateXDegrees( float degreesAboutX )
{
	AppendTransform( CreateXRotationDegrees( degreesAboutX ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::RotateYDegrees( float degreesAboutY )
{
	AppendTransform( CreateYRotationDegrees( degreesAboutY ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::RotateZDegrees( float degreesAboutZ )
{
	AppendTransform( CreateZRotationDegrees( degreesAboutZ ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::Translate2D( const Vec2& translationXY )
{
	AppendTransform( CreateTranslation2D( translationXY ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::Translate3D( const Vec3& translation3D )
{
	AppendTransform( CreateTranslation3D( translation3D ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::ScaleUniform2D( float scaleFactorXY )
{
	AppendTransform( CreateUniformScale2D( scaleFactorXY ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::ScaleNonUniform2D( const Vec2& scaleFactorsXY )
{
	AppendTransform( CreateNonUniformScale2D( scaleFactorsXY ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::ScaleUniform3D( float uniformScaleXYZ )
{
	AppendTransform( CreateUniformScale3D( uniformScaleXYZ ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::ScaleNonUniform3D( const Vec3& scaleFactorsXYZ )
{
	AppendTransform( CreateNonUniformScale3D( scaleFactorsXYZ ) );
}


//-----------------------------------------------------------------------------------------------
void Mat44::AppendTransform( const Mat44& transformationToConcatenate )
{
	// Nicknames for brevity
	Mat44 old(*this);
	const Mat44& in = transformationToConcatenate;

	Ix = ( old.Ix * in.Ix ) + ( old.Jx * in.Iy ) + ( old.Kx * in.Iz ) + ( old.Tx * in.Iw );
	Jx = ( old.Ix * in.Jx ) + ( old.Jx * in.Jy ) + ( old.Kx * in.Jz ) + ( old.Tx * in.Jw );
	Kx = ( old.Ix * in.Kx ) + ( old.Jx * in.Ky ) + ( old.Kx * in.Kz ) + ( old.Tx * in.Kw );
	Tx = ( old.Ix * in.Tx ) + ( old.Jx * in.Ty ) + ( old.Kx * in.Tz ) + ( old.Tx * in.Tw );

	Iy = ( old.Iy * in.Ix ) + ( old.Jy * in.Iy ) + ( old.Ky * in.Iz ) + ( old.Ty * in.Iw );
	Jy = ( old.Iy * in.Jx ) + ( old.Jy * in.Jy ) + ( old.Ky * in.Jz ) + ( old.Ty * in.Jw );
	Ky = ( old.Iy * in.Kx ) + ( old.Jy * in.Ky ) + ( old.Ky * in.Kz ) + ( old.Ty * in.Kw );
	Ty = ( old.Iy * in.Tx ) + ( old.Jy * in.Ty ) + ( old.Ky * in.Tz ) + ( old.Ty * in.Tw );

	Iz = ( old.Iz * in.Ix ) + ( old.Jz * in.Iy ) + ( old.Kz * in.Iz ) + ( old.Tz * in.Iw );
	Jz = ( old.Iz * in.Jx ) + ( old.Jz * in.Jy ) + ( old.Kz * in.Jz ) + ( old.Tz * in.Jw );
	Kz = ( old.Iz * in.Kx ) + ( old.Jz * in.Ky ) + ( old.Kz * in.Kz ) + ( old.Tz * in.Kw );
	Tz = ( old.Iz * in.Tx ) + ( old.Jz * in.Ty ) + ( old.Kz * in.Tz ) + ( old.Tz * in.Tw );

	Iw = ( old.Iw * in.Ix ) + ( old.Jw * in.Iy ) + ( old.Kw * in.Iz ) + ( old.Tw * in.Iw );
	Jw = ( old.Iw * in.Jx ) + ( old.Jw * in.Jy ) + ( old.Kw * in.Jz ) + ( old.Tw * in.Jw );
	Kw = ( old.Iw * in.Kx ) + ( old.Jw * in.Ky ) + ( old.Kw * in.Kz ) + ( old.Tw * in.Kw );
	Tw = ( old.Iw * in.Tx ) + ( old.Jw * in.Ty ) + ( old.Kw * in.Tz ) + ( old.Tw * in.Tw );
}


//-----------------------------------------------------------------------------------------------
void Mat44::Transpose()
{
	float* matrixData = GetAsFloatArray();

	for ( int yIndex = 0; yIndex < 4; ++yIndex )
	{
		for ( int xIndex = 0; xIndex < 4; ++xIndex )
		{
			if ( yIndex == xIndex )
			{
				continue;
			}

			float temp = matrixData[yIndex * 4 + xIndex];
			matrixData[yIndex * 4 + xIndex] = matrixData[xIndex * 4 + yIndex];
			matrixData[xIndex * 4 + yIndex] = temp;
		}
	}
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateXRotationDegrees( float degreesAboutX )
{
	Mat44 newMatrix;
	float cosine = CosDegrees( degreesAboutX );
	float sine = SinDegrees( degreesAboutX );

	newMatrix.Jy = cosine;
	newMatrix.Ky = -sine;

	newMatrix.Jz = sine;
	newMatrix.Kz = cosine;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateYRotationDegrees( float degreesAboutY )
{
	Mat44 newMatrix;
	float cosine = CosDegrees( degreesAboutY );
	float sine = SinDegrees( degreesAboutY );

	newMatrix.Ix = cosine;
	newMatrix.Kx = sine;

	newMatrix.Iz = -sine;
	newMatrix.Kz = cosine;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateZRotationDegrees( float degreesAboutZ )
{
	Mat44 newMatrix;
	float cosine = CosDegrees( degreesAboutZ );
	float sine = SinDegrees( degreesAboutZ );

	newMatrix.Ix = cosine;
	newMatrix.Jx = -sine;

	newMatrix.Iy = sine;
	newMatrix.Jy = cosine;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateRotationFromPitchRollYawDegrees( float pitch, float roll, float yaw )
{
	/*Mat44 rotationMatrix = CreateXRotationDegrees( pitch );
	rotationMatrix.RotateYDegrees( yaw );
	rotationMatrix.RotateZDegrees( roll );*/

	//Mat44 rotationMatrix = CreateYRotationDegrees( yaw );
	//rotationMatrix.RotateXDegrees( pitch );
	//rotationMatrix.RotateZDegrees( roll );
	Mat44 rotationMatrix;
	rotationMatrix.AppendTransform( CreateXRotationDegrees( pitch ) );
	rotationMatrix.AppendTransform( CreateZRotationDegrees( roll ) );
	rotationMatrix.AppendTransform( CreateYRotationDegrees( yaw ) );

	return rotationMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateXYZRotationDegrees( const Vec3& rotation )
{
	return CreateRotationFromPitchRollYawDegrees( rotation.x, rotation.y, rotation.z );
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateTranslation2D( const Vec2& translationXY )
{
	Mat44 newMatrix;

	newMatrix.Tx = translationXY.x;
	newMatrix.Ty = translationXY.y;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateTranslation3D( const Vec3& translation3D )
{
	Mat44 newMatrix;

	newMatrix.Tx = translation3D.x;
	newMatrix.Ty = translation3D.y;
	newMatrix.Tz = translation3D.z;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateUniformScale2D( float scaleFactorXY )
{
	Mat44 newMatrix;
	
	newMatrix.Ix = scaleFactorXY;
	newMatrix.Jy = scaleFactorXY;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateNonUniformScale2D( const Vec2& scaleFactorsXY )
{
	Mat44 newMatrix;

	newMatrix.Ix = scaleFactorsXY.x;
	newMatrix.Jy = scaleFactorsXY.y;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateUniformScale3D( float scaleFactorsXYZ )
{
	Mat44 newMatrix;

	newMatrix.Ix = scaleFactorsXYZ;
	newMatrix.Jy = scaleFactorsXYZ;
	newMatrix.Kz = scaleFactorsXYZ;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateNonUniformScale3D( const Vec3& scaleFactorsXYZ )
{
	Mat44 newMatrix;

	newMatrix.Ix = scaleFactorsXYZ.x;
	newMatrix.Jy = scaleFactorsXYZ.y;
	newMatrix.Kz = scaleFactorsXYZ.z;

	return newMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreateOrthographicProjection( const Vec3& min, const Vec3& max )
{
	// think of x
	// min.x, max.x -> (-1,1)
	// ndc.x = ((x -  min.x) / (max.x - min.x)) * (( 1.f - (-1.f)) + -1.f )
	// ndc.x = x / (max.x - min.x) - (min.x / (max.x - min.x)) * 2.f + -1.f )
	// a = (max.x - min.x)
	// b = (-2.f * min.x - max.x + min.x) / (max.x - min.x)
	//   = -(max.x - min.x) / (max.x - min.x)

	// min.z, max.z -> (0,1) (In DirectX only, map to -1, 1 in other APIs)
	// ndc.z = ((z -  min.z) / (max.z - min.z)) * (( 1.f - (0.f)) + 0.f )
	// ndc.z = z / (max.z - min.z) - (min.z / (max.z - min.z)) * 1.f )
	// a = 1.f / (max.z - min.z)
	// b = -min.z / (max.z - min.z)


	Vec3 diff = max - min;
	Vec3 sum = max + min;

	float projMatrix[] =
	{
		2.f / diff.x,		0.f,				0.f,				0.f,
		0.f,				2.f / diff.y,		0.f,				0.f,
		0.f,				0.f,				1.f / diff.z,		0.f,
		-sum.x / diff.x,	-sum.y / diff.y,	-min.z / diff.z,	1.f
	};

	return Mat44( projMatrix );
}


//-----------------------------------------------------------------------------------------------
const Mat44 Mat44::CreatePerspectiveProjection( float fovDegrees,
												float aspectRatio,
												float nearZ, float farZ )
{
	// float fovDegrees is the field of view you want
	// float farZ and nearZ are the depth range you want to see
	// -> do **not** span zero here.

	// Goal is to...
	// - setup a default "depth" where (1, 1) == (1, 1) after proejction (1 / tan(fov * .5f))
	// - map z to w, so the z divide happens
	// - map nearZ to 0, farZ to farZ, since a Z divide will happen
	//   and this will result in mapping nearZ to 0, and farZ to 1. 
	//   -> ((z - nz) / (fz - nz)) * fz + 0
	//   -> -fz / (fz - nz) * z      + (fz * nz) / (fz - nz)

	float height = 1.0f / TanDegrees( fovDegrees * .5f ); // how far away are we for the perspective point to be "one up" from our forward line. 
	float zRange = farZ - nearZ;
	float inverseZRange = 1.0f / zRange;

	float projMatrix[] =
	{
		height / aspectRatio,	0.f,		0.f,								0.f,
		0.f,					height,		0.f,								0.f,
		0.f,					0.f,		-farZ * inverseZRange,				-1.f,
		0.f,					0.f,		nearZ * farZ * inverseZRange,		0.f
	};

	return Mat44( projMatrix );
}

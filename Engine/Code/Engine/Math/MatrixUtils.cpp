#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
Mat44 MakeOrthographicProjectionMatrixD3D( float minX, float maxX, float minY, float maxY, float minZ, float maxZ )
{
	Vec3 mins( minX, minY, minZ );
	Vec3 maxs( maxX, maxY, maxZ );
	return Mat44::CreateOrthographicProjection( mins, maxs );
}

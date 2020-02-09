#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Mat44 MakeOrthographicProjectionMatrixD3D( float minX, float maxX, float minY, float maxY, float minZ, float maxZ )
{
	float d3dMinX = RangeMapFloat( minX, maxX, -1.f, 1.f, minX );
	float d3dMaxX = RangeMapFloat( minX, maxX, -1.f, 1.f, maxX );
	return Mat44();
}

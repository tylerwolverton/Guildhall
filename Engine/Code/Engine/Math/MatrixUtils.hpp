#pragma once
#include "Engine/Math/Mat44.hpp"


//-----------------------------------------------------------------------------------------------
// create a projection matrix that maps the cube given to the D3D NDC cube
Mat44 MakeOrthographicProjectionMatrixD3D( float minX, float maxX, 
										   float minY, float maxY, 
										   float minZ, float maxZ );

Mat44 MakePerspectiveProjectionMatrixD3D( float fovDegrees, 
										  float aspectRatio, 
										  float nearZ, float farZ );

void TransposeMatrix( Mat44& matrix );
bool IsOrthoNormalMatrix( Mat44& matrix );
void InvertOrthoNormalMatrix( Mat44& matrix );
void InvertMatrix( Mat44& matrix );

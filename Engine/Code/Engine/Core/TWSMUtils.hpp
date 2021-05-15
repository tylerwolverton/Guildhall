#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class CPUMesh;


//-----------------------------------------------------------------------------------------------
void SaveMeshAsTWSMFile( const CPUMesh& mesh, const std::string& filename );
CPUMesh* LoadTWSMFileIntoCPUMesh( const std::string& filename );

#pragma once
#include <string>

class GPUMesh;
class RenderContext;

//-----------------------------------------------------------------------------------------------
class OBJLoader
{
public:
	GPUMesh* LoadFromFile( RenderContext* context, std::string filename );

private:
};

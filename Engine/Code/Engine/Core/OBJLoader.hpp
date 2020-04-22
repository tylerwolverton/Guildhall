#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vec3;
class GPUMesh;
class RenderContext;


//-----------------------------------------------------------------------------------------------
struct Face
{
	int position = -1;
	int normal = -1;
	int uv = -1;
};


//-----------------------------------------------------------------------------------------------
class OBJLoader
{
public:
	GPUMesh* LoadFromFile( RenderContext* context, std::string filename );

private:
	bool AppendVertexData( const Strings& dataStrings, std::vector<Vec3>& data );
	bool AppendFace( const Strings& dataStrings, std::vector<Face>& data, const Face& lastFace );
};

#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vec3;
class GPUMesh;
class RenderContext;


//-----------------------------------------------------------------------------------------------
struct ObjVertex
{
	int position = -1;
	int normal = -1;
	int uv = -1;
};


//-----------------------------------------------------------------------------------------------
struct ObjFace
{
public:
	ObjVertex vertices[3];
};


//-----------------------------------------------------------------------------------------------
class OBJLoader
{
public:
	GPUMesh* LoadFromFile( RenderContext* context, std::string filename );

private:
	bool AppendVertexData( const Strings& dataStrings, std::vector<Vec3>& data );
	bool AppendVertexUVs( const Strings& dataStrings, std::vector<Vec3>& data );
	bool AppendFace( const Strings& dataStrings, std::vector<ObjFace>& data, ObjVertex& lastObjVertex );

	ObjVertex CreateObjVertexFromString( const std::string& indexStr, ObjVertex& lastObjVertex );
};

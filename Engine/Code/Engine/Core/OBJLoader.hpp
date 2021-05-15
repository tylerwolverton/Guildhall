#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Mat44.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vec3;
struct Vertex_PCUTBN;
class RenderContext;


//-----------------------------------------------------------------------------------------------
struct ObjVertex
{
	int position = -1;
	int normal = -1;
	int uv = -1;
};


//-----------------------------------------------------------------------------------------------
struct OrientationMetaData
{
	Mat44 orientationMatrix;
	bool invertWindingOrder = false;
};


//-----------------------------------------------------------------------------------------------
struct ObjFace
{
public:
	ObjVertex vertices[3];
};


//-----------------------------------------------------------------------------------------------
class ObjLoader
{
public:	
	static void LoadFromFile( std::vector<Vertex_PCUTBN>& vertices,
							  const std::string& filename,
							  bool& out_fileHadNormals );

	static void InvertVertVs( std::vector<Vertex_PCUTBN>& vertices );
	static void GenerateVertNormals( std::vector<Vertex_PCUTBN>& vertices );
	static void InvertVertWindingOrder( std::vector<Vertex_PCUTBN>& vertices );
	static void InvertIndexWindingOrder( std::vector<uint>& indices );
	static void GenerateVertTangents( std::vector<Vertex_PCUTBN>& vertices );
	static void TransformVerts( std::vector<Vertex_PCUTBN>& vertices, const Mat44& transform );
	static void CleanMesh( std::vector<Vertex_PCUTBN>& vertices, std::vector<uint>& indices );

private:
	static bool AppendVertexData( const Strings& dataStrings, std::vector<Vec3>& data );
	static bool AppendVertexUVs( const Strings& dataStrings, std::vector<Vec3>& data );
	static bool AppendFace( const Strings& dataStrings, std::vector<ObjFace>& data, ObjVertex& lastObjVertex );

	static bool ParseMetadata( const Strings& dataStrings, int lineNum, Mat44& scaleTransform, OrientationMetaData& orientationTransform );
	static Vec3 GetVecForRelativeDir( const std::string& relativeDir );

	static ObjVertex CreateObjVertexFromString( const std::string& indexStr, ObjVertex& lastObjVertex );
};

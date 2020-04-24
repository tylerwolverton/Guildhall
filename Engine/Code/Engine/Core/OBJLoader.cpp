#include "Engine/Core/ObjLoader.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "ThirdParty/mikkt/mikktspace.h"

#include <iostream>
#include <fstream>


//-----------------------------------------------------------------------------------------------
void ObjLoader::LoadFromFile( std::vector<Vertex_PCUTBN>& vertices,
							  std::string filename,
							  bool& out_fileHadNormals )
{
	std::string line;
	std::ifstream objFile;
	objFile.open( filename, std::ios::in );

	if ( !objFile.is_open() )
	{
		// Load error model
		return;
	}

	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec3> uvTexCoords;
	std::vector<ObjFace> faces;
	ObjVertex lastObjVertex;
	while ( std::getline( objFile, line ) )
	{
		line = TrimOuterWhitespace( line );
		
		if ( IsEmptyOrWhitespace( line )
			 || line[0] == '#' )
		{
			continue;
		}

		Strings dataStrings = SplitStringOnDelimiter( line, ' ' );
		dataStrings = TrimOuterWhitespace( dataStrings );

		if ( dataStrings[0] == "mtllib" )
		{
			// TODO: Handle materials
		}
		else if( dataStrings[0] == "v" )
		{
			AppendVertexData( dataStrings, positions );
		}
		else if ( dataStrings[0] == "vn" )
		{
			AppendVertexData( dataStrings, normals );
		}
		else if ( dataStrings[0] == "vt" )
		{
			AppendVertexUVs( dataStrings, uvTexCoords );
		}
		else if ( dataStrings[0] == "f" )
		{
			AppendFace( dataStrings, faces, lastObjVertex );
		}
	}

	objFile.close();

	for( uint faceIdx = 0; faceIdx < faces.size(); ++faceIdx )
	{
		const ObjFace& face = faces[faceIdx];
		Vertex_PCUTBN faceVertices[3];
		for ( int faceVertIdx = 0; faceVertIdx < 3; ++faceVertIdx )
		{
			Vertex_PCUTBN& vertex = faceVertices[faceVertIdx];
			vertex.position = positions[face.vertices[faceVertIdx].position];
			int uvIdx = face.vertices[faceVertIdx].uv;
			if( uvIdx != -1 )
			{
				vertex.uvTexCoords = uvTexCoords[uvIdx].XY();
			}

			int normalIdx = face.vertices[faceVertIdx].normal;
			if ( normalIdx != -1 )
			{
				vertex.normal = normals[normalIdx].GetNormalized();
			}
			
			vertices.push_back( vertex );
		}

		out_fileHadNormals = normals.size() != 0;
	}
}


//-----------------------------------------------------------------------------------------------
bool ObjLoader::AppendVertexData( const Strings& dataStrings, std::vector<Vec3>& data )
{
	if ( dataStrings.size() != 4 )
	{
		g_devConsole->PrintString( Stringf( "Unsupported number of vertices: %d", dataStrings.size() - 1 ), Rgba8::YELLOW );
		return false;
	}

	data.push_back( Vec3( (float)atof( dataStrings[1].c_str() ),
						  (float)atof( dataStrings[2].c_str() ), 
						  (float)atof( dataStrings[3].c_str() ) ) );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ObjLoader::AppendVertexUVs( const Strings& dataStrings, std::vector<Vec3>& data )
{
	if ( dataStrings.size() < 3 
		 || dataStrings.size() > 4 )
	{
		g_devConsole->PrintString( Stringf( "Unsupported number of vertices: %d", dataStrings.size() - 1 ), Rgba8::YELLOW );
		return false;
	}

	float u = ConvertStringToFloat( dataStrings[1] );
	float v = ConvertStringToFloat( dataStrings[2] );

	float w = 0.f;
	if( dataStrings.size() == 4 )
	{
		w = ConvertStringToFloat( dataStrings[3] );
	}

	data.push_back( Vec3( u, v, w ) );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ObjLoader::AppendFace( const Strings& dataStrings, std::vector<ObjFace>& data, ObjVertex& lastObjVertex )
{
	if ( dataStrings.size() < 4 
		 || dataStrings.size() > 5 )
	{
		g_devConsole->PrintString( Stringf( "Unsupported number of faces: %d", dataStrings.size() - 1 ), Rgba8::YELLOW );
		return false;
	}

	ObjVertex vert0( CreateObjVertexFromString( dataStrings[1], lastObjVertex ) );
	ObjVertex vert1( CreateObjVertexFromString( dataStrings[2], lastObjVertex ) );
	ObjVertex vert2( CreateObjVertexFromString( dataStrings[3], lastObjVertex ) );

	ObjFace face0;
	face0.vertices[0] = vert0;
	face0.vertices[1] = vert1;
	face0.vertices[2] = vert2;
	
	data.push_back( face0 );

	if( dataStrings.size() == 5 )
	{
		ObjVertex vert3( CreateObjVertexFromString( dataStrings[4], lastObjVertex ) );

		ObjFace face1;
		face1.vertices[0] = vert0;
		face1.vertices[1] = vert2;
		face1.vertices[2] = vert3;

		data.push_back( face1 );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
// Face entries will always be in the form v/vt/vn with both vt and vn being optional
//-----------------------------------------------------------------------------------------------
ObjVertex ObjLoader::CreateObjVertexFromString( const std::string& indexStr, ObjVertex& lastObjVertex )
{
	Strings indicesStr = SplitStringOnDelimiter( indexStr, '/' );

	ObjVertex newVertex;
	// Subtract 1 to line up the indices with arrays
	if( !indicesStr[0].empty() )
	{
		newVertex.position = ConvertStringToInt( indicesStr[0] ) - 1;
		lastObjVertex.position = newVertex.position;
	}

	// Check if we have texture coords
	if( indicesStr.size() > 1 )
	{
		if ( !indicesStr[1].empty() )
		{
			lastObjVertex.uv = ConvertStringToInt( indicesStr[1] ) - 1;
		}

		newVertex.uv = lastObjVertex.uv;
	}

	// Check if we have normals
	if ( indicesStr.size() > 2 )
	{
		if ( !indicesStr[2].empty() )
		{
			lastObjVertex.normal = ConvertStringToInt( indicesStr[2] ) - 1;
		}

		newVertex.normal = lastObjVertex.normal;
	}
		
	return newVertex;
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::InvertVertVs( std::vector<Vertex_PCUTBN>& vertices )
{
	for ( uint vertIdx = 0; vertIdx < vertices.size(); ++vertIdx )
	{
		Vertex_PCUTBN& vert = vertices[vertIdx];
		vert.uvTexCoords.y = 1.f - vert.uvTexCoords.y;
	}
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::GenerateVertNormals( std::vector<Vertex_PCUTBN>& vertices )
{
	for ( uint vertIdx = 0; vertIdx < vertices.size() - 3; vertIdx += 3 )
	{
		Vertex_PCUTBN& vert0 = vertices[vertIdx];
		Vertex_PCUTBN& vert1 = vertices[vertIdx + 1];
		Vertex_PCUTBN& vert2 = vertices[vertIdx + 2];

		Vec3 edge0 = vert1.position - vert0.position;
		Vec3 edge1 = vert2.position - vert0.position;

		Vec3 normal = CrossProduct3D( edge0, edge1 );
		vert0.normal = normal;
		vert1.normal = normal;
		vert2.normal = normal;
	}
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::InvertVertWindingOrder( std::vector<Vertex_PCUTBN>& vertices )
{
	for ( uint vertIdx = 0; vertIdx < vertices.size() - 3; vertIdx += 3 )
	{
		Vertex_PCUTBN temp = vertices[vertIdx];
		vertices[vertIdx] = vertices[vertIdx + 2];
		vertices[vertIdx + 2] = temp;
	}
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::InvertIndexWindingOrder( std::vector<uint>& indices )
{
	for ( uint indexIdx = 0; indexIdx < indices.size() - 3; indexIdx += 3 )
	{
		uint temp = indices[indexIdx];
		indices[indexIdx] = indices[indexIdx + 2];
		indices[indexIdx + 2] = temp;
	}
}


//-----------------------------------------------------------------------------------------------
// Mikkelson Tangents
//-----------------------------------------------------------------------------------------------
static int GetNumFaces( SMikkTSpaceContext const* context )
{
	// if you had index buffer
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	return (int)vertices.size() / 3;
}


//-----------------------------------------------------------------------------------------------
static int GetNumberOfVerticesForFace( SMikkTSpaceContext const* context,
									   const int iFace )
{
	UNUSED( context );
	UNUSED( iFace );

	return 3;
}


//-----------------------------------------------------------------------------------------------
static void GetPositionForFaceVert( const SMikkTSpaceContext* context,
									float fvPosOut[],
									const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	int indexInVertexArray = iFace * 3 + iVert;

	Vec3 outPos = vertices[indexInVertexArray].position;

	fvPosOut[0] = outPos.x;
	fvPosOut[1] = outPos.y;
	fvPosOut[2] = outPos.z;
}


//-----------------------------------------------------------------------------------------------
static void GetNormalForFaceVert( const SMikkTSpaceContext* context,
								  float fvNormOut[],
								  const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	int indexInVertexArray = iFace * 3 + iVert;

	Vec3 outNormal = vertices[indexInVertexArray].normal;

	fvNormOut[0] = outNormal.x;
	fvNormOut[1] = outNormal.y;
	fvNormOut[2] = outNormal.z;
}


//-----------------------------------------------------------------------------------------------
static void GetUVForFaceVert( const SMikkTSpaceContext* context,
							  float fvTexcOut[],
							  const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	int indexInVertexArray = iFace * 3 + iVert;

	Vec2 outNormal = vertices[indexInVertexArray].uvTexCoords;

	fvTexcOut[0] = outNormal.x;
	fvTexcOut[1] = outNormal.y;
}


//-----------------------------------------------------------------------------------------------
static void SetTangent( const SMikkTSpaceContext* context,
						const float fvTangent[],
						const float fSign,
						const int iFace, const int iVert )
{
	std::vector<Vertex_PCUTBN>& vertices = *( std::vector<Vertex_PCUTBN>* )( context->m_pUserData );
	int indexInVertexArray = iFace * 3 + iVert;


	vertices[indexInVertexArray].tangent = Vec3( fvTangent[0], fvTangent[1], fvTangent[2] ).GetNormalized();

	vertices[indexInVertexArray].bitangent = CrossProduct3D( vertices[indexInVertexArray].tangent,
															 vertices[indexInVertexArray].normal ) * fSign;
}


//-----------------------------------------------------------------------------------------------
// Assume I'm using only vertex array - not indexed array
static void GenerateTangentsForVertexArray( std::vector<Vertex_PCUTBN>& vertices )
{
	SMikkTSpaceInterface mikktInterface;

	// How does MikkT get info
	mikktInterface.m_getNumFaces = GetNumFaces;
	mikktInterface.m_getNumVerticesOfFace = GetNumberOfVerticesForFace;

	mikktInterface.m_getPosition = GetPositionForFaceVert;
	mikktInterface.m_getNormal = GetNormalForFaceVert;
	mikktInterface.m_getTexCoord = GetUVForFaceVert;

	// MikkT telling US info
	mikktInterface.m_setTSpaceBasic = SetTangent;
	mikktInterface.m_setTSpace = nullptr;

	// Next, the context!  
	// Encapsulate ONE instance of running the algorithm
	SMikkTSpaceContext context;
	context.m_pInterface = &mikktInterface;
	context.m_pUserData = &vertices;

	// RUN THE ALGO
	genTangSpaceDefault( &context );
}


//-----------------------------------------------------------------------------------------------
void ObjLoader::GenerateVertTangents( std::vector<Vertex_PCUTBN>& vertices )
{
	GenerateTangentsForVertexArray( vertices );
}

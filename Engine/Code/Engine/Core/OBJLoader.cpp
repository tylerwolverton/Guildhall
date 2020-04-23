#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/GPUMesh.hpp"

#include <iostream>
#include <fstream>


//-----------------------------------------------------------------------------------------------
GPUMesh* OBJLoader::LoadFromFile( RenderContext* context, std::string filename )
{
	std::string line;
	std::ifstream objFile;
	objFile.open( filename, std::ios::in );

	if ( !objFile.is_open() )
	{
		// Load error model
		return nullptr;
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

	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	uint index = 0;
	for( uint faceIdx = 0; faceIdx < faces.size(); ++faceIdx )
	{
		const ObjFace& face = faces[faceIdx];

		for ( int faceVertIdx = 0; faceVertIdx < 3; ++faceVertIdx )
		{
			Vertex_PCUTBN vertex;
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

			vertex.tangent = Vec3( 0.f, 1.f, 0.f );
			vertex.bitangent = CrossProduct3D( vertex.normal, vertex.tangent );

			vertices.push_back( vertex );
			indices.push_back( index++ );
		}
	}

	return new GPUMesh( context, vertices, indices );
}


//-----------------------------------------------------------------------------------------------
bool OBJLoader::AppendVertexData( const Strings& dataStrings, std::vector<Vec3>& data )
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
bool OBJLoader::AppendVertexUVs( const Strings& dataStrings, std::vector<Vec3>& data )
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
bool OBJLoader::AppendFace( const Strings& dataStrings, std::vector<ObjFace>& data, ObjVertex& lastObjVertex )
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
ObjVertex OBJLoader::CreateObjVertexFromString( const std::string& indexStr, ObjVertex& lastObjVertex )
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


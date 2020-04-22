#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
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

	std::vector<Vertex_PCUTBN> vertices;
	std::vector<uint> indices;
	std::vector<Vec3> positions;
	std::vector<Vec3> normals;
	std::vector<Vec3> textureCoords;
	std::vector<Face> faces;
	Face lastFace;
	while ( std::getline( objFile, line ) )
	{
		line = TrimOuterWhitespace( line );
		
		if ( IsEmptyOrWhitespace( line )
			 || line[0] == '#' )
		{
			continue;
		}

		Strings dataStrings = SplitStringOnDelimiterAndTrimOuterWhitespace( line, ' ' );
		int numDataElements = dataStrings.size();

		if ( dataStrings[0] == "mtllib" )
		{

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
			AppendVertexData( dataStrings, textureCoords );
		}
		else if ( dataStrings[0] == "f" )
		{
			if ( AppendFace( dataStrings, faces, lastFace ) )
			{
				lastFace = *faces.end();
			}
		}
	}

	objFile.close();

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
bool OBJLoader::AppendFace( const Strings& dataStrings, std::vector<Face>& data, const Face& lastFace )
{
	if ( dataStrings.size() < 4 
		 || dataStrings.size() > 5 )
	{
		g_devConsole->PrintString( Stringf( "Unsupported number of faces: %d", dataStrings.size() - 1 ), Rgba8::YELLOW );
		return false;
	}

	/*indices.push_back( (uint)atoi( dataStrings[1].c_str() ) - 1 );
	indices.push_back( (uint)atoi( dataStrings[2].c_str() ) - 1 );
	indices.push_back( (uint)atoi( dataStrings[3].c_str() ) - 1 );*/
	
	return true;
}


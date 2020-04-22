#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
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
	while ( std::getline( objFile, line ) )
	{
		// TODO: Make SplitStringsOnWhiteSpace?
		Strings dataStrings = SplitStringOnDelimiter( line, ' ' );
		int numDataElements = dataStrings.size();

		if ( numDataElements == 0 )
		{
			continue;
		}

		if( dataStrings[0] == "v" )
		{
			if ( numDataElements != 4 )
			{
				g_devConsole->PrintString( Stringf( "Unsupported number of vertices: %d", numDataElements - 1 ), Rgba8::YELLOW );
				continue;
			}

			Vec3 position( (float)atof( dataStrings[1].c_str() ), (float)atof( dataStrings[2].c_str() ), (float)atof( dataStrings[3].c_str() ) );

			Vertex_PCUTBN vertex;
			vertex.position = position;

			vertices.push_back( vertex );
		}
		else if ( dataStrings[0] == "f" )
		{
			if ( numDataElements != 4 )
			{
				g_devConsole->PrintString( Stringf( "Unsupported number of faces: %d", numDataElements - 1 ), Rgba8::YELLOW );
				continue;
			}
						
			indices.push_back( (uint)atoi( dataStrings[1].c_str() ) - 1 );
			indices.push_back( (uint)atoi( dataStrings[2].c_str() ) - 1 );
			indices.push_back( (uint)atoi( dataStrings[3].c_str() ) - 1 );
		}
	}

	objFile.close();

	return new GPUMesh( context, vertices, indices );
}

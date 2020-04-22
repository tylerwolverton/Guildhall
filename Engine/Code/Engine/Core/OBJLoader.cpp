#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

#include <iostream>
#include <fstream>


//-----------------------------------------------------------------------------------------------
void OBJLoader::LoadFromFile( std::string filename )
{
	std::string line;
	std::ifstream objFile;
	objFile.open( filename, std::ios::in );

	if ( !objFile.is_open() )
	{
		// Load error model
		return;
	}

	std::vector<Vertex_PCUTBN> vertices;
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


		}
		else if ( dataStrings[0] == "f" )
		{

		}
	}

	objFile.close();

}

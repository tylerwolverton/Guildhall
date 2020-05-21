#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <iostream>
#include <fstream>


//-----------------------------------------------------------------------------------------------
void* FileReadToNewBuffer( const std::string& filename, size_t* out_fileSize )
{
	FILE* fp;
	fopen_s( &fp, filename.c_str(), "r" );
	if ( fp == nullptr )
	{
		return nullptr;
	}

	// get size of file
	fseek( fp, 0, SEEK_END );
	long fileSize = ftell( fp );

	unsigned char* buffer = new unsigned char[(size_t)fileSize + 1];
	// Guarantee or die not null
	if ( buffer != nullptr )
	{
		fseek( fp, 0, SEEK_SET );
		size_t bytesRead = fread( buffer, 1, fileSize, fp );
		buffer[bytesRead] = NULL;
	}

	if ( out_fileSize != nullptr )
	{
		*out_fileSize = (size_t)fileSize;
	}

	fclose( fp );

	return buffer;
}


//-----------------------------------------------------------------------------------------------
Strings SplitFileIntoLines( const std::string& filename )
{
	Strings lines;
	std::ifstream objFile;
	objFile.open( filename, std::ios::in );

	if ( !objFile.is_open() )
	{
		g_devConsole->PrintString( Stringf( "Couldn't open file to split: '%s'", filename.c_str(), Rgba8::RED ) );
		return lines;
	}

	std::string line;
	while ( std::getline( objFile, line ) )
	{
		lines.push_back( line );
	}

	objFile.close();

	return lines;
}

#include "Engine/Core/FileUtils.hpp"


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

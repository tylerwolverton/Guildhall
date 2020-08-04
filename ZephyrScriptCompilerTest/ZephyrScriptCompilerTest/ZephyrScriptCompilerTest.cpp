#include <iostream>
#include <stdio.h>
#include <fstream>

#include "Chunk.hpp"


//-----------------------------------------------------------------------------------------------
int main()
{
	/*FILE* scriptFile = nullptr;
	scriptFile = std::fopen_s( "./test.zs", "r" );

	if ( scriptFile != nullptr )
	{
		fprintf( scriptFile, "" );
		fclose( scriptFile );
	}

    std::cout << "Hello World!\n";*/

	Chunk testChunk( "chunk1" );
	testChunk.WriteByte( eOpCode::OP_RETURN );
	testChunk.WriteByte( eOpCode::OP_RETURN );
	testChunk.WriteByte( eOpCode::OP_RETURN );

	testChunk.Disassemble();
}

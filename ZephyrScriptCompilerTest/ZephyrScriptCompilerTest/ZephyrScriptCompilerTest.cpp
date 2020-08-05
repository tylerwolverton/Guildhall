#include <iostream>
#include <stdio.h>
#include <fstream>

#include "Chunk.hpp"
#include "VirtualMachine.hpp"


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
	testChunk.WriteByte( eOpCode::OP_CONSTANT );
	testChunk.WriteConstant( Value( 4.f ) );

	testChunk.WriteByte( eOpCode::OP_CONSTANT );
	testChunk.WriteConstant( Value( 2.f ) );

	testChunk.WriteByte( eOpCode::OP_MULTIPLY );

	testChunk.WriteByte( eOpCode::OP_CONSTANT );
	testChunk.WriteConstant( Value( 2.f ) );

	testChunk.WriteByte( eOpCode::OP_ADD );

	testChunk.WriteByte( eOpCode::OP_RETURN );

	testChunk.Disassemble();

	VirtualMachine vm;
	vm.Interpret( testChunk );
}

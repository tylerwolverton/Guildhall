#include <iostream>
#include <fstream>
#include <sstream>

#include "Chunk.hpp"
#include "VirtualMachine.hpp"


//-----------------------------------------------------------------------------------------------
bool RunFromFile( const char* script )
{
	// read file into memory
	std::ifstream inFile( script );
	if ( !inFile.is_open() )
	{
		std::cout << "Input file couldn't be opened\n";
		return false;
	}

	std::stringstream buffer;
	buffer << inFile.rdbuf();
	inFile.close();
	
	VirtualMachine vm;
	eInterpretResult result = vm.Interpret( buffer.str() );

	if ( result == eInterpretResult::COMPILE_ERROR ) exit( 65 );
	if ( result == eInterpretResult::RUNTIME_ERROR ) exit( 70 );

	return true;
}


//-----------------------------------------------------------------------------------------------
int main()
{
	if ( !RunFromFile( "./test.zs" ) )
	{
		return 1;
	}

	/*Chunk testChunk( "chunk1" );
	testChunk.WriteByte( eOpCode::CONSTANT );
	testChunk.WriteConstant( Value( 4.f ) );

	testChunk.WriteByte( eOpCode::CONSTANT );
	testChunk.WriteConstant( Value( 2.f ) );

	testChunk.WriteByte( eOpCode::MULTIPLY );

	testChunk.WriteByte( eOpCode::CONSTANT );
	testChunk.WriteConstant( Value( 2.f ) );

	testChunk.WriteByte( eOpCode::ADD );

	testChunk.WriteByte( eOpCode::RETURN );

	testChunk.Disassemble();

	VirtualMachine vm;
	vm.Interpret( testChunk );*/
}

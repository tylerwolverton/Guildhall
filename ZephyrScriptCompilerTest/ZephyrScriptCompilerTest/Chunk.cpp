#include "Chunk.hpp"

#include <iostream>


//-----------------------------------------------------------------------------------------------
Chunk::Chunk( const std::string& name )
	: m_name( name )
{
}


//-----------------------------------------------------------------------------------------------
void Chunk::WriteByte( eOpCode opCode )
{
	m_bytes.push_back( (byte)opCode );
}


//-----------------------------------------------------------------------------------------------
void Chunk::Disassemble()
{
	std::cout << "----" << m_name << "----" << std::endl;

	for ( int offsetToNextInstruction = 0; offsetToNextInstruction < (int)m_bytes.size(); /* don't increment here, DisassembleInstruction will do it*/ )
	{
		offsetToNextInstruction = DisassembleInstruction( offsetToNextInstruction );
	}
}


//-----------------------------------------------------------------------------------------------
int Chunk::DisassembleInstruction( int offsetToNextInstruction )
{
	std::cout << offsetToNextInstruction << " ";

	eOpCode opCode = ByteToOpCode( m_bytes[offsetToNextInstruction] );
	switch ( opCode )
	{
		case eOpCode::OP_RETURN:
		{
			std::cout << "OP_RETURN\n";
			return offsetToNextInstruction + 1;
			break;
		}
		default:
		{
			std::cout << "Unknown opcode '" << m_bytes[offsetToNextInstruction] << "'\n";
			return offsetToNextInstruction + 1;
		}
	}
}

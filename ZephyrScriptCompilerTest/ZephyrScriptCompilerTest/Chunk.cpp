#include "Chunk.hpp"

#include <iostream>


//-----------------------------------------------------------------------------------------------
Chunk::Chunk( const std::string& name )
	: m_name( name )
{
}


//-----------------------------------------------------------------------------------------------
void Chunk::WriteByte( byte newByte )
{
	m_bytes.push_back( newByte );
}


//-----------------------------------------------------------------------------------------------
void Chunk::WriteByte( eOpCode opCode )
{
	WriteByte( (byte)opCode );
}


//-----------------------------------------------------------------------------------------------
void Chunk::WriteByte( int constantIdx )
{
	WriteByte( (byte)constantIdx );
}


//-----------------------------------------------------------------------------------------------
void Chunk::WriteConstant( const Value& constant )
{
	WriteByte( AddConstant( constant ) );
}


//-----------------------------------------------------------------------------------------------
int Chunk::AddConstant( const Value& constant )
{
	m_constants.push_back( constant );
	return (int)m_constants.size() - 1;
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
		case eOpCode::OP_CONSTANT:
		{
			byte constantIdx = m_bytes[offsetToNextInstruction + 1];
			std::cout << ToString( opCode ) << " " << (int)constantIdx <<" '" << m_constants[constantIdx].value << "'\n";
			return offsetToNextInstruction + 2;
		}

		case eOpCode::OP_ADD:
		case eOpCode::OP_SUBTRACT:
		case eOpCode::OP_MULTIPLY:
		case eOpCode::OP_DIVIDE:
		case eOpCode::OP_NEGATE:
		case eOpCode::OP_RETURN:
		{
			std::cout << ToString( opCode ) << std::endl;
			return offsetToNextInstruction + 1;
		}

		default:
		{
			std::cout << "Unknown opcode '" << m_bytes[offsetToNextInstruction] << "'\n";
			return offsetToNextInstruction + 1;
		}
	}
}

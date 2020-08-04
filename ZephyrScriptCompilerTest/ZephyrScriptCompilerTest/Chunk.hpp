#pragma once
#include "BytecodeCommon.hpp"


//-----------------------------------------------------------------------------------------------
class Chunk
{
public:
	Chunk( const std::string& name );

	void WriteByte( eOpCode opCode );

	void Disassemble();

private:
	int DisassembleInstruction( int offsetToNextInstruction );

private:
	std::string m_name;
	std::vector<byte> m_bytes;
};

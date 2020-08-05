#pragma once
#include "BytecodeCommon.hpp"


//-----------------------------------------------------------------------------------------------
class Chunk
{
public:
	Chunk( const std::string& name );

	std::vector<byte> GetCode() const { return m_bytes; }
	byte GetByte( int idx ) const { return m_bytes[idx]; }
	Value GetConstant( int idx ) const { return m_constants[idx]; }

	void WriteByte( byte newByte );
	void WriteByte( eOpCode opCode );
	void WriteByte( int constantIdx );
	void WriteConstant( const Value& constant );

	int AddConstant( const Value& constant );

	void Disassemble();

private:
	int DisassembleInstruction( int offsetToNextInstruction );

private:
	std::string m_name;
	std::vector<byte> m_bytes;
	std::vector<Value> m_constants;
};

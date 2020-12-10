#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"


class ZephyrBytecodeChunk
{
public:
	ZephyrBytecodeChunk( const std::string& name );

	std::vector<byte> GetCode() const							{ return m_bytes; }
	byte GetByte( int idx ) const								{ return m_bytes[idx]; }
	float GetNumberConstant( int idx ) const					{ return m_numberConstants[idx]; }

	void WriteByte( byte newByte );
	void WriteByte( eOpCode opCode );
	void WriteByte( int constantIdx );
	void WriteNumberConstant( float constant );

	int AddNumberConstant( float constant );

	void Disassemble();

private:
	std::string m_name;
	std::vector<byte> m_bytes;
	std::vector<float> m_numberConstants;
};

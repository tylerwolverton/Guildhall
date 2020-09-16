#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk
{
public:
	ZephyrBytecodeChunk( const std::string& name );

	// Accessors
	std::vector<byte>	GetCode() const									{ return m_bytes; }
	int					GetNumBytes() const								{ return (int)m_bytes.size(); }
	byte				GetByte( int idx ) const						{ return m_bytes[idx]; }
	float				GetNumberConstant( int idx ) const				{ return m_numberConstants[idx]; }

	// Methods to write data to chunk
	void WriteByte( byte newByte );
	void WriteByte( eOpCode opCode );
	void WriteByte( int constantIdx );
	void WriteNumberConstant( NUMBER_TYPE constant );
	int AddNumberConstant( NUMBER_TYPE constant );

	// Debug methods
	void Disassemble();

private:
	std::string m_name;
	std::vector<byte> m_bytes;
	std::vector<NUMBER_TYPE> m_numberConstants;
};

#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <map>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk
{
public:
	ZephyrBytecodeChunk( const std::string& name );

	// Accessors
	std::vector<byte>	GetCode() const									{ return m_bytes; }
	int					GetNumBytes() const								{ return (int)m_bytes.size(); }
	byte				GetByte( int idx ) const						{ return m_bytes[idx]; }
	ZephyrValue			GetConstant( int idx ) const					{ return m_constants[idx]; }
	bool				TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const;
	ZephyrValueMap		GetVariables() const							{ return m_variables; }
	ZephyrValueMap*		GetUpdateableVariables() 						{ return &m_variables; }

	// Methods to write data to chunk
	void WriteByte( byte newByte );
	void WriteByte( eOpCode opCode );
	void WriteByte( int constantIdx );
	void WriteConstant( const ZephyrValue& constant );
	int AddConstant( const ZephyrValue& constant );

	void SetVariable( const std::string& identifier, const ZephyrValue& value );

	// Debug methods
	void Disassemble();

private:
	std::string m_name;
	std::vector<byte> m_bytes;
	std::vector<ZephyrValue> m_constants;
	ZephyrValueMap m_variables;
};

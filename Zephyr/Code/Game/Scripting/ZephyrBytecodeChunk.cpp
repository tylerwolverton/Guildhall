#include "Game/Scripting/ZephyrBytecodeChunk.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk::ZephyrBytecodeChunk( const std::string& name )
	: m_name( name )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::WriteByte( byte newByte )
{
	m_bytes.push_back( newByte );
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::WriteByte( eOpCode opCode )
{
	WriteByte( (byte)opCode );
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::WriteByte( int constantIdx )
{
	//if ( constantIdx < 0
	//	 || constantIdx > sizeof( byte ) - 1 )
	//{
	//	// Print error?
	//	return;
	//}

	WriteByte( (byte)constantIdx );
}


//-----------------------------------------------------------------------------------------------
int ZephyrBytecodeChunk::WriteNumberConstant( float constant )
{
	m_numberConstants.push_back( constant );
	return (int)m_numberConstants.size() - 1;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::Disassemble()
{
	// Write debug code to print out chunks
}



#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk::ZephyrBytecodeChunk( const std::string& name )
	: m_name( name )
{
}


//-----------------------------------------------------------------------------------------------
bool ZephyrBytecodeChunk::TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const
{
	auto variableEntry = m_variables.find( identifier );
	if ( variableEntry == m_variables.end() )
	{
		return false;
	}

	out_value = variableEntry->second;
	return true;
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
void ZephyrBytecodeChunk::WriteConstant( const ZephyrValue& constant )
{
	WriteByte( AddConstant( constant ) );
}


//-----------------------------------------------------------------------------------------------
int ZephyrBytecodeChunk::AddConstant( const ZephyrValue& constant )
{
	m_constants.push_back( constant );
	return (int)m_constants.size() - 1;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::SetVariable( const std::string& identifier, const ZephyrValue& value )
{
	m_variables[identifier] = value;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::Disassemble()
{
	// Write debug code to print out chunks
}



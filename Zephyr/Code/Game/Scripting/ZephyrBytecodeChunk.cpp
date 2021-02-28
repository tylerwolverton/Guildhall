#include "Game/Scripting/ZephyrBytecodeChunk.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk::ZephyrBytecodeChunk( const std::string& name, ZephyrBytecodeChunk* parent )
	: m_name( name )
	, m_parentChunk( parent )
{
}


//-----------------------------------------------------------------------------------------------
bool ZephyrBytecodeChunk::TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const
{
	auto variableEntry = m_variables.find( identifier );
	if ( variableEntry != m_variables.end() )
	{
		out_value = variableEntry->second;
		return true;
	}

	if ( m_parentChunk != nullptr )
	{
		return m_parentChunk->TryToGetVariable( identifier, out_value );
	}

	return false;
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
void ZephyrBytecodeChunk::SetConstantAtIdx( int idx, const ZephyrValue& constant )
{
	if ( idx > (int)m_constants.size() - 1 )
	{
		g_devConsole->PrintWarning( "Tried to write to constant outside bounds of constant vector" );
		return;
	}

	m_constants[idx] = constant;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::AddEventChunk( ZephyrBytecodeChunk* eventBytecodeChunk )
{
	m_eventBytecodeChunks[eventBytecodeChunk->GetName()] = eventBytecodeChunk;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::SetVariable( const std::string& identifier, const ZephyrValue& value )
{
	m_variables[identifier] = value;
}

//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::SetVec2Variable( const std::string& identifier, const std::string& memberName, const ZephyrValue& value )
{
	Vec2 var = m_variables[identifier].GetAsVec2();
	if ( memberName == "x" )
	{
		var.x = value.GetAsNumber();
	}
	if ( memberName == "y" )
	{
		var.y = value.GetAsNumber();
	}

	m_variables[identifier] = ZephyrValue( var );
}

//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::Disassemble()
{
	// Write debug code to print out chunks
}


//-----------------------------------------------------------------------------------------------
std::string ToString( eBytecodeChunkType type )
{
	switch ( type )
	{
		case eBytecodeChunkType::STATE_MACHINE:		return "Global State";
		case eBytecodeChunkType::STATE:				return "State";
		case eBytecodeChunkType::EVENT:				return "Function";
	}

	return "Unknown";
}

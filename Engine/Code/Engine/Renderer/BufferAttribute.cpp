#include "Engine/Renderer/BufferAttribute.hpp"


//-----------------------------------------------------------------------------------------------
BufferAttribute::BufferAttribute( const char* name, eBufferFormatType bufferFormatType, uint offset )
	: m_name( name )
	, m_bufferFormatType( bufferFormatType )
	, m_offset( offset )
{

}

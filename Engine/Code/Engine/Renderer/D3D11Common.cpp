#include "Engine/Renderer/D3D11Common.hpp"


//-----------------------------------------------------------------------------------------------
D3D11_USAGE ToDxMemoryUsage( eRenderMemoryHint hint )
{
	switch ( hint )
	{
		case MEMORY_HINT_GPU: return D3D11_USAGE_DEFAULT;
		case MEMORY_HINT_DYNAMIC: return D3D11_USAGE_DYNAMIC;
		case MEMORY_HINT_STAGING: return D3D11_USAGE_STAGING;
		default: ERROR_AND_DIE( "Unknown hint" ); return D3D11_USAGE_DEFAULT;
	}
}


//-----------------------------------------------------------------------------------------------
UINT ToDXUsage( eRenderBufferUsage usage )
{
	uint ret = 0;

	if ( usage & VERTEX_BUFFER_BIT )
	{
		ret |= D3D11_BIND_VERTEX_BUFFER;
	}

	if ( usage & INDEX_BUFFER_BIT )
	{
		ret |= D3D11_BIND_INDEX_BUFFER;
	}

	if ( usage & UNIFORM_BUFFER_BIT )
	{
		ret |= D3D11_BIND_CONSTANT_BUFFER;
	}

	return ret;
}

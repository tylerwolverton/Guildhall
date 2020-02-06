#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
RenderBuffer::RenderBuffer( RenderContext* owner, eRenderBufferUsage usage, eRenderMemoryHint memHint )
	: m_owner( owner )
	, m_usage( usage )
	, m_memHint( memHint )
{
	
}


//-----------------------------------------------------------------------------------------------
RenderBuffer::~RenderBuffer()
{
	DX_SAFE_RELEASE( m_handle );
}


//-----------------------------------------------------------------------------------------------
bool RenderBuffer::Update( const void* data, size_t dataByteSize, size_t elementByteSize )
{
	// 1. if not compatible - destroy the old buffer
	// our elementSize matches the passed in 
	// if we're GPU 
		// bufferSizes must match
	// if we're dynamic
		// passed in buffer size is less than our bufferSize
	if ( !IsCompatible( dataByteSize, elementByteSize ) )
	{
		Cleanup(); // destroy the handle, reset things
	}

	// 2. if no buffer, create one that is compatible
	if ( m_handle == nullptr )
	{
		if ( !Create( dataByteSize, elementByteSize ) )
		{
			return false;
		}
	}

	GUARANTEE_OR_DIE( m_handle != nullptr, "Invalid handle for render buffer" );

	// 3. updating the buffer
	ID3D11DeviceContext* context = m_owner->m_context;
	if ( m_memHint == MEMORY_HINT_DYNAMIC )
	{
		// Mapping
		// Only available to dynamic buffer
		// But don't have to reallocate if going smaller
		D3D11_MAPPED_SUBRESOURCE mapped;
		
		HRESULT result = context->Map( m_handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped );
		if ( SUCCEEDED( result ) )
		{
			memcpy( mapped.pData, data, dataByteSize );
			context->Unmap( m_handle, 0 );
		}
		else
		{
			return false;
		}
	}
	else
	{
		// if this is MEMORY_HINT_GPU
		// Copy Subresource (direct copy)
		// This is only available to GPU buffers that have exactly the same size and element size
		// Need to recreate if size changes
		context->UpdateSubresource( m_handle, 0, nullptr, data, 0, 0 );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool RenderBuffer::Create( size_t dataByteSize, size_t elementByteSize )
{
	ID3D11Device* device = m_owner->m_device;

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (uint)dataByteSize;
	desc.Usage = ToDxMemoryUsage( m_memHint );
	desc.BindFlags = ToDXUsage( m_usage );
	
	desc.CPUAccessFlags = 0;
	if ( m_memHint == MEMORY_HINT_DYNAMIC )
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if ( m_memHint == MEMORY_HINT_STAGING )
	{
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}

	desc.MiscFlags = 0;
	desc.StructureByteStride = (uint)elementByteSize;

	device->CreateBuffer( &desc, nullptr, &m_handle );

	if ( m_handle == nullptr )
	{
		return false;
	}

	m_bufferByteSize = dataByteSize;
	m_elementByteSize = elementByteSize;
	
	return true;
}


//-----------------------------------------------------------------------------------------------
bool RenderBuffer::IsCompatible( size_t dataByteSize, size_t elementByteSize ) const
{
	if ( m_handle == nullptr )
	{
		return false;
	}

	if ( m_elementByteSize != elementByteSize )
	{
		return false;
	}

	if ( m_memHint == MEMORY_HINT_DYNAMIC )
	{
		return dataByteSize <= m_bufferByteSize;
	}
	else
	{
		return dataByteSize == m_bufferByteSize;
	}
}


//-----------------------------------------------------------------------------------------------
void RenderBuffer::Cleanup()
{
	DX_SAFE_RELEASE( m_handle );
	m_bufferByteSize = 0;
	m_elementByteSize = 0;
}

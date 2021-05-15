#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/BufferAttribute.hpp"


//-----------------------------------------------------------------------------------------------
D3D11_USAGE ToDxMemoryUsage( eRenderMemoryHint hint )
{
	switch ( hint )
	{
		case MEMORY_HINT_GPU: return D3D11_USAGE_DEFAULT;
		case MEMORY_HINT_DYNAMIC: return D3D11_USAGE_DYNAMIC;
		case MEMORY_HINT_STAGING: return D3D11_USAGE_STAGING;
		default: ERROR_AND_DIE( "Unknown hint" );
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


//-----------------------------------------------------------------------------------------------
DXGI_FORMAT ToDXGIFormat( eBufferFormatType formatType )
{
	switch ( formatType )
	{
		case BUFFER_FORMAT_VEC2: return DXGI_FORMAT_R32G32_FLOAT;
		case BUFFER_FORMAT_VEC3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case BUFFER_FORMAT_VEC4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case BUFFER_FORMAT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case BUFFER_FORMAT_INT: return DXGI_FORMAT_R32_SINT;
		case BUFFER_FORMAT_UNKNOWN: 
		default:
			ERROR_AND_DIE( "Unknown DXGI format type" );
	}
}


//-----------------------------------------------------------------------------------------------
D3D11_COMPARISON_FUNC ToDxComparisonFunc( eCompareFunc funcType )
{
	switch ( funcType )
	{
		case eCompareFunc::COMPARISON_NEVER: return D3D11_COMPARISON_NEVER;
		case eCompareFunc::COMPARISON_LESS: return D3D11_COMPARISON_LESS;
		case eCompareFunc::COMPARISON_LESS_EQUAL: return D3D11_COMPARISON_LESS_EQUAL;
		case eCompareFunc::COMPARISON_GREATER: return D3D11_COMPARISON_GREATER;
		case eCompareFunc::COMPARISON_GREATER_EQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
		case eCompareFunc::COMPARISON_EQUAL: return D3D11_COMPARISON_EQUAL;
		case eCompareFunc::COMPARISON_ALWAYS: return D3D11_COMPARISON_ALWAYS;
	}

	ERROR_AND_DIE( "Unknown compare func type" );
}


//-----------------------------------------------------------------------------------------------
D3D11_FILL_MODE ToDXFillMode( eFillMode fillMode )
{
	switch ( fillMode )
	{
		case eFillMode::WIREFRAME: return D3D11_FILL_WIREFRAME;
		case eFillMode::SOLID: return D3D11_FILL_SOLID;
	}

	ERROR_AND_DIE( "Unsupported fill mode" );
}


//-----------------------------------------------------------------------------------------------
eFillMode FromDXFillMode( D3D11_FILL_MODE fillMode )
{
	switch ( fillMode )
	{
		case D3D11_FILL_WIREFRAME: return eFillMode::WIREFRAME;
		case D3D11_FILL_SOLID: return eFillMode::SOLID;
	}

	ERROR_AND_DIE( "Unsupported fill mode" );
}


//-----------------------------------------------------------------------------------------------
D3D11_CULL_MODE ToDXCullMode( eCullMode cullMode )
{
	switch ( cullMode )
	{
		case eCullMode::NONE: return D3D11_CULL_NONE;
		case eCullMode::FRONT: return D3D11_CULL_FRONT;
		case eCullMode::BACK: return D3D11_CULL_BACK;
	}

	ERROR_AND_DIE( "Unsupported cull mode" );
}


//-----------------------------------------------------------------------------------------------
eCullMode FromDXCullMode( D3D11_CULL_MODE cullMode )
{
	switch ( cullMode )
	{
		case D3D11_CULL_NONE: return eCullMode::NONE;
		case D3D11_CULL_FRONT: return eCullMode::FRONT;
		case D3D11_CULL_BACK: return  eCullMode::BACK;
	}

	ERROR_AND_DIE( "Unsupported cull mode" );
}

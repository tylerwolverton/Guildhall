#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include <d3dcompiler.h>


//-----------------------------------------------------------------------------------------------
Shader::Shader( RenderContext* owner )
	: m_owner( owner )
{
	CreateRasterState();
}


//-----------------------------------------------------------------------------------------------
Shader::~Shader()
{
	DX_SAFE_RELEASE( m_rasterState );
}


//-----------------------------------------------------------------------------------------------
bool Shader::CreateFromFile( const std::string& filename )
{
	size_t fileSize = 0;
	void* source = FileReadToNewBuffer( filename, &fileSize );
	if ( source == nullptr )
	{
		return false;
	}

	m_vertexStage.Compile( m_owner, filename, source, fileSize, SHADER_TYPE_VERTEX );
	m_fragmentStage.Compile( m_owner, filename, source, fileSize, SHADER_TYPE_FRAGMENT );

	delete[] source;

	return true;
}


//-----------------------------------------------------------------------------------------------
void Shader::CreateRasterState()
{
	D3D11_RASTERIZER_DESC desc;

	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_NONE;
	desc.FrontCounterClockwise = TRUE; // the only reason we're doing this; 
	desc.DepthBias = 0U;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = TRUE;
	desc.ScissorEnable = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;

	ID3D11Device* device = m_owner->m_device;
	device->CreateRasterizerState( &desc, &m_rasterState );
}


//-----------------------------------------------------------------------------------------------
ShaderStage::ShaderStage()
{

}


//-----------------------------------------------------------------------------------------------
ShaderStage::~ShaderStage()
{
	DX_SAFE_RELEASE( m_bytecode );
	DX_SAFE_RELEASE( m_handle );
}


//-----------------------------------------------------------------------------------------------
static const char* GetDefaultEntryPointForStage( eShaderType stage )
{
	switch ( stage )
	{
		case SHADER_TYPE_VERTEX: return "VertexFunction";
		case SHADER_TYPE_FRAGMENT: return "FragmentFunction";
		default: GUARANTEE_OR_DIE(false, "Unknown shader stage");
	}
}


//-----------------------------------------------------------------------------------------------
static const char* GetShaderModelForStage( eShaderType stage )
{
	switch ( stage )
	{
		case SHADER_TYPE_VERTEX: return "vs_5_0";
		case SHADER_TYPE_FRAGMENT: return "ps_5_0";
		default: GUARANTEE_OR_DIE( false, "Unknown shader stage" );
	}
}


//-----------------------------------------------------------------------------------------------
bool ShaderStage::Compile( RenderContext* renderContext, const std::string& filename, const void* source, const size_t sourceByteLength, eShaderType stage )
{
	// HLSL High Level Shading Language
	// HLSL -> Bytecode
	// Link Bytecode -> Device Assembly (target language) - This is device specific

	const char* entryPoint = GetDefaultEntryPointForStage( stage );
	const char* shaderModel = GetShaderModelForStage( stage );

	DWORD compileFlags = 0U;
	#if defined(DEBUG_SHADERS)
		compileFlags |= D3DCOMPILE_DEBUG;
		compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
		compileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;   // cause, FIX YOUR WARNINGS
	#else 
		// compile_flags |= D3DCOMPILE_SKIP_VALIDATION;   // Only do this if you know for a fact this shader works with this device (so second run through of a game)
		compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;   // Yay, fastness (default is level 1)
	#endif

	ID3DBlob* bytecode = nullptr;
	ID3DBlob* errors = nullptr;

	HRESULT hr = ::D3DCompile( source,
							   sourceByteLength,
							   filename.c_str(),
							   nullptr,
							   D3D_COMPILE_STANDARD_FILE_INCLUDE, // include rules - 
							   entryPoint,
							   shaderModel,
							   compileFlags,
							   0,
							   &bytecode, // can be cached to improve load times on subsequent loads
							   &errors );


	if ( FAILED( hr ) || ( errors != nullptr ) ) 
	{
		if ( errors != nullptr ) 
		{
			char* error_string = (char*)errors->GetBufferPointer();
			DebuggerPrintf( "Failed to compile [%s].  Compiler gave the following output;\n%s",
							 filename.c_str(),
							 error_string );

			DEBUGBREAK();
		}	
	}
	else
	{
		ID3D11Device* device = renderContext->m_device;
		const void* bytecodePtr = bytecode->GetBufferPointer();
		size_t bytecodeSize = bytecode->GetBufferSize();
		switch ( stage )
		{
			case SHADER_TYPE_VERTEX:
			{
				hr = device->CreateVertexShader( bytecodePtr, bytecodeSize, nullptr, &m_vertexShader );
				GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Failed to link vertex shader stage" );
			} break;
			case SHADER_TYPE_FRAGMENT:
			{
				hr = device->CreatePixelShader( bytecodePtr, bytecodeSize, nullptr, &m_fragmentShader );
				GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Failed to link fragment shader stage" );
			} break;

			default: GUARANTEE_OR_DIE( false, "Unimplemented stage." ); break;
		}
	}

	DX_SAFE_RELEASE( errors );
	
	if ( stage == SHADER_TYPE_VERTEX )
	{
		m_bytecode = bytecode;
	}
	else
	{
		DX_SAFE_RELEASE( bytecode );
		m_bytecode = nullptr;
	}

	m_type = stage;

	return IsValid();
}

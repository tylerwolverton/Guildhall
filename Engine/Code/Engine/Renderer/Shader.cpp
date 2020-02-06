#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/BufferAttribute.hpp"
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
	DX_SAFE_RELEASE( m_inputLayout );
}


//-----------------------------------------------------------------------------------------------
bool Shader::CreateFromFile( const std::string& fileName )
{
	size_t fileSize = 0;
	void* source = FileReadToNewBuffer( fileName, &fileSize );
	if ( source == nullptr )
	{
		return false;
	}

	m_vertexStage.Compile( m_owner, fileName, source, fileSize, SHADER_TYPE_VERTEX );
	m_fragmentStage.Compile( m_owner, fileName, source, fileSize, SHADER_TYPE_FRAGMENT );

	delete[] source;

	m_fileName = fileName;
	
	return true;
}




//-----------------------------------------------------------------------------------------------
ID3D11InputLayout* Shader::GetOrCreateInputLayout( const BufferAttribute* attributes )
{
	if ( m_inputLayout != nullptr 
		 || attributes == m_lastLayout )
	{
		return m_inputLayout;
	}

	m_vertexDesc.clear();
	PopulateVertexDescription( attributes );

	ID3D11Device* device = m_owner->m_device;
	device->CreateInputLayout( m_vertexDesc.data(), (uint)m_vertexDesc.size(),					// describe vertex
							   m_vertexStage.GetBytecode(), m_vertexStage.GetBytecodeLength(),	// describe shader
							   &m_inputLayout );

	return m_inputLayout;
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
void Shader::PopulateVertexDescription( const BufferAttribute* attributes )
{
	int vertexDescIdx = 0;
	while ( attributes[vertexDescIdx].m_bufferFormatType != BUFFER_FORMAT_UNKNOWN )
	{
		D3D11_INPUT_ELEMENT_DESC desc;
		desc.SemanticName = attributes[vertexDescIdx].m_name.c_str();					// semantic name in shader of the data we're binding to; 
		desc.SemanticIndex = 0;															// If you have an array, which index of the area are we binding to
		desc.Format = ToDXGIFormat( attributes[vertexDescIdx].m_bufferFormatType );		// What data is here
		desc.InputSlot = 0;																// Which input slot is the data coming from (where you bind your stream)
		desc.AlignedByteOffset = attributes[vertexDescIdx].m_offset;					// where the data appears from start of a vertex
		desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;								// type of data (vertex or instance)
		desc.InstanceDataStepRate = 0;													// used in instance rendering to describe when we move this data forward

		m_vertexDesc.push_back( desc );

		++vertexDescIdx;
	}

	m_lastLayout = attributes;
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

	HRESULT hr = ::D3DCompile( source,								// plain text source code
							   sourceByteLength,					
							   filename.c_str(),					// optional, used for error messages (If you HLSL has includes - it will not use the includes names, it will use this name)
							   nullptr,								// pre-compiler defines - used more for compiling multiple versions of a single shader (different quality specs, or shaders that are mostly the same outside some constants)
							   D3D_COMPILE_STANDARD_FILE_INCLUDE,   // include rules - this allows #includes in the shader to work relative to the src_file path or my current working directly
							   entryPoint,							// Entry Point for this shader
							   shaderModel,							// Compile Target (MSDN - "Specifying Compiler Targets")
							   compileFlags,						// Flags that control compilation
							   0,									// Effect Flags (we will not be doing Effect Files)
							   &bytecode,							// [OUT] ID3DBlob (buffer) that will store the byte code. ( can be cached to improve load times on subsequent loads )
							   &errors );							// [OUT] ID3DBlob (buffer) that will store error information


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

//-----------------------------------------------------------------------------------------------
const void* ShaderStage::GetBytecode() const
{
	return m_bytecode->GetBufferPointer();
}


//-----------------------------------------------------------------------------------------------
size_t ShaderStage::GetBytecodeLength() const
{
	return m_bytecode->GetBufferSize();
}

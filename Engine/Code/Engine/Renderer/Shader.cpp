#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
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
	DX_SAFE_RELEASE( m_inputLayout );
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
ID3D11InputLayout* Shader::GetOrCreateInputLayout( )
{
	if ( m_inputLayout != nullptr )
	{
		return m_inputLayout;
	}

	ID3D11Device* device = m_owner->m_device;


	D3D11_INPUT_ELEMENT_DESC vertexDesc[3];

	// position
	vertexDesc[0].SemanticName				= "POSITION";							// semantic name in shader of the data we're bindnig to; 
	vertexDesc[0].SemanticIndex				= 0;									// If you have an array, which index of the area are we binding to
	vertexDesc[0].Format					= DXGI_FORMAT_R32G32B32_FLOAT;			// What data is here - 3 32-bit floats
	vertexDesc[0].InputSlot					= 0;									// Which input slot is the data coming from (where you bind your stream)
	vertexDesc[0].AlignedByteOffset			= offsetof( Vertex_PCU, m_position );	// where the data appears from start of a vertex
	vertexDesc[0].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;			// type of data (vertex or instance)
	vertexDesc[0].InstanceDataStepRate		= 0;									// used in instance rendering to describe when we move this data forward

	// color
	vertexDesc[1].SemanticName				= "COLOR";								// semantic name in shader of the data we're bindnig to; 
	vertexDesc[1].SemanticIndex				= 0;									
	vertexDesc[1].Format					= DXGI_FORMAT_R8G8B8A8_UNORM;			// 4 1-byte channel, unsigned normal value ( converts between 0-1 )
	vertexDesc[1].InputSlot					= 0;									
	vertexDesc[1].AlignedByteOffset			= offsetof( Vertex_PCU, m_color );		
	vertexDesc[1].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;			
	vertexDesc[1].InstanceDataStepRate		= 0;									

	// uv
	vertexDesc[2].SemanticName				= "TEXCOORD";							// semantic name in shader of the data we're bindnig to; 
	vertexDesc[2].SemanticIndex				= 0;									
	vertexDesc[2].Format					= DXGI_FORMAT_R32G32_FLOAT;				// 2 32-bit floats
	vertexDesc[2].InputSlot					= 0;									
	vertexDesc[2].AlignedByteOffset			= offsetof( Vertex_PCU, m_uvTexCoords );		
	vertexDesc[2].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;			
	vertexDesc[2].InstanceDataStepRate		= 0;									

	device->CreateInputLayout( vertexDesc, 3,													// describe vertex
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

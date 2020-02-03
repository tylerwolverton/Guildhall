#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
struct ID3D11Resource;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D10Blob;
struct ID3D11RasterizerState;
class RenderContext;
class VertexBuffer;


//-----------------------------------------------------------------------------------------------
enum eShaderType
{
	SHADER_TYPE_VERTEX,
	SHADER_TYPE_FRAGMENT,
};


//-----------------------------------------------------------------------------------------------
class ShaderStage
{
public:
	ShaderStage();
	~ShaderStage();

	// A02
	bool Compile( RenderContext* renderContext,
							   const std::string& filename,
							   const void* source,
							   const size_t sourceByteLength,
							   eShaderType stage );

	inline bool IsValid() const { return m_handle != nullptr; }


public:
	eShaderType m_type = eShaderType::SHADER_TYPE_VERTEX;
	ID3D10Blob* m_bytecode = nullptr;

	union
	{
		ID3D11Resource* m_handle = nullptr;
		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader* m_fragmentShader;
	};
};


//-----------------------------------------------------------------------------------------------
class Shader
{
public:
	Shader( RenderContext* owner );
	~Shader();

	bool CreateFromFile( const std::string& filename );

	// for hooking IA (input assembler) to the VS (vertex shader), 
	// needs to vertex shader and vertex format to make the binding
	ID3D11InputLayout* GetOrCreateInputLayout( VertexBuffer* vbo );            // A02

	void CreateRasterState();

public:
	RenderContext* m_owner;
	ShaderStage m_vertexStage;
	ShaderStage m_fragmentStage;

	ID3D11InputLayout* m_inputLayout = nullptr; // for now, we'll have 1, but in the future you could have one for each different vertex type you use with this
	ID3D11RasterizerState* m_rasterState = nullptr;
};

#pragma once
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct ID3D11Resource;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D10Blob;
struct ID3D11RasterizerState;
struct D3D11_INPUT_ELEMENT_DESC;
struct BufferAttribute;
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

	const void* GetBytecode() const;
	size_t GetBytecodeLength() const;

public:
	eShaderType m_type = eShaderType::SHADER_TYPE_VERTEX;

	union
	{
		ID3D11Resource* m_handle = nullptr;
		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader* m_fragmentShader;
	};

private:
	ID3D10Blob* m_bytecode = nullptr;
};


//-----------------------------------------------------------------------------------------------
class Shader
{
public:
	Shader( RenderContext* owner );
	~Shader();

	bool CreateFromFile( const std::string& filename );

	// for hooking IA (input assembler) to the VS (vertex shader), 
	// needs the vertex shader and vertex format to make the binding
	ID3D11InputLayout* GetOrCreateInputLayout( const BufferAttribute* attributes );            // A02

	void CreateRasterState();

private:
	void PopulateVertexDescription( const BufferAttribute* attributes );            // A02

public:
	RenderContext* m_owner;
	ShaderStage m_vertexStage;
	ShaderStage m_fragmentStage;

	ID3D11InputLayout* m_inputLayout = nullptr; // for now, we'll have 1, but in the future you could have one for each different vertex type you use with this
	ID3D11RasterizerState* m_rasterState = nullptr;

private:
	const BufferAttribute*					m_lastLayout = nullptr;
	std::vector<D3D11_INPUT_ELEMENT_DESC>	m_vertexDesc;
};

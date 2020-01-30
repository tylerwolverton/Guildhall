#pragma once
#include <string>

//-----------------------------------------------------------------------------------------------
struct ID3D11Resource;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D10Blob;
struct ID3D11RasterizerState;
class RenderContext;


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
							   const std::string& filename, // east west const situation
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
	void CreateRasterState();

public:
	RenderContext* m_owner;
	ShaderStage m_vertexStage;
	ShaderStage m_fragmentStage;

	ID3D11RasterizerState* m_rasterState = nullptr;
};

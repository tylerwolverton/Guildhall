#pragma once
#include <string>

//-----------------------------------------------------------------------------------------------
struct ID3D11Resource;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
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
	bool LoadShaderFromSource( RenderContext* renderContext,
							   const std::string& filename, // east west const situation
							   const void* source,
							   const size_t sourceByteLength,
							   eShaderType stage );

	inline bool IsValid() const { return m_handle != nullptr; }

public:
	eShaderType m_type = eShaderType::SHADER_TYPE_VERTEX;
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
	Shader();
	~Shader();
	bool CreateFromFile( const std::string& filename );

public:
	ShaderStage m_vertexStage;
	ShaderStage m_fragmentStage;
};

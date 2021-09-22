#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/D3D11/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
class ShaderProgram;


//-----------------------------------------------------------------------------------------------
struct ShaderState
{
	bool isWindingCCW = true;
	eCullMode cullMode = eCullMode::NONE;
	eFillMode fillMode = eFillMode::SOLID;

	eBlendMode blendMode = eBlendMode::DISABLED;

	eCompareFunc depthTestCompare = eCompareFunc::COMPARISON_ALWAYS;
	bool writeDpeth = false;
};


//-----------------------------------------------------------------------------------------------
class Shader
{
public:
	Shader() = default;
	explicit Shader( RenderContext* context, const char* filename );

	std::string GetName()								{ return m_name; }
	const char* GetFileName()							{ return m_filename.c_str(); }
	ShaderProgram* GetShaderProgram()					{ return m_program; }
	ShaderState GetShaderState()						{ return m_state; }

	void SetShaderProgram( ShaderProgram* program )		{ m_program = program; }

private:
	RenderContext* m_context = nullptr;
	std::string m_filename;
	std::string m_name;
	
	ShaderProgram* m_program = nullptr;
	ShaderState m_state;
};

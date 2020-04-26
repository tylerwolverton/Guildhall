#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
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
	explicit Shader( RenderContext* context, const char* filename, const XmlElement& shaderElem );

	std::string GetName()							{ return m_name; }
	const char* GetFileName()						{ return m_filename; }
	ShaderProgram* GetShaderProgram()				{ return m_program; }
	ShaderState GetShaderState()					{ return m_state; }

private:
	RenderContext* m_context = nullptr;
	const char* m_filename;
	std::string m_name;
	
	ShaderProgram* m_program = nullptr;
	ShaderState m_state;
};

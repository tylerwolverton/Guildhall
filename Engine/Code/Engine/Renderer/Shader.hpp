#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"


class Shader
{
public:
	explicit Shader( const XmlElement& shaderElem );

	ShaderProgram* GetShaderProgram() { return m_program; }

private:
	ShaderProgram* m_program;
};

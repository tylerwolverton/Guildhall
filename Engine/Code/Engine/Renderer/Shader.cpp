#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include <d3dcompiler.h>


//-----------------------------------------------------------------------------------------------
Shader::Shader()
{

}


//-----------------------------------------------------------------------------------------------
Shader::~Shader()
{

}


//-----------------------------------------------------------------------------------------------
bool Shader::CreateFromFile( const std::string& filename )
{
	size_t fileSize = 0;
	void* src = FileReadToNewBuffer( filename, &fileSize );
	if ( src == nullptr )
	{
		return false;
	}

	delete[] src;

	return true;
}


//-----------------------------------------------------------------------------------------------
ShaderStage::ShaderStage()
{

}


//-----------------------------------------------------------------------------------------------
ShaderStage::~ShaderStage()
{

}

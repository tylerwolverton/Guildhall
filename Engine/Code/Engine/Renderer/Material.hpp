#pragma once
#include "Engine/Core/XmlUtils.hpp"


//-----------------------------------------------------------------------------------------------
class RenderContext;
class Sampler;
class Shader;
class Texture;


//-----------------------------------------------------------------------------------------------
class Material
{
public:
	Material( RenderContext* context, const char* filename, const XmlElement& materialElem );

private:
	RenderContext* m_context = nullptr;
	const char* m_filename;
	std::string m_name;

	Shader* m_shader = nullptr;

	Texture* m_diffuseTexture = nullptr;
	Texture* m_normalTexture = nullptr;

	Sampler* m_defaultSampler = nullptr;

	std::vector<Texture*> m_userTextures;
	std::vector<Sampler*> m_userSamplers;

	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;
};

#pragma once
#include <vector>

class Shader;
class Texture;

class Material
{
public:
	void Bind();
	
	void SetShader( Shader* shader )							{ m_shader = shader; }
	void SetDiffuseTexture( Texture* diffuseTexture )			{ m_diffuseTexture = diffuseTexture; }
	void SetNormalTexture( Texture* normalTexture )				{ m_normalTexture = normalTexture; }
	//void SetUserTexture( int slot, Texture* texture );

	float GetSpecularFactor() { return m_specularFactor; }
	float GetSpecularPower() { return m_specularPower; }


private:
	Shader* m_shader = nullptr;
	Texture* m_diffuseTexture = nullptr;
	Texture* m_normalTexture = nullptr;
	std::vector<Texture*> m_userTextures;

	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;
};
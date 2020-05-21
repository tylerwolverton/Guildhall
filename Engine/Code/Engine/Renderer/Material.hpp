#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"

//-----------------------------------------------------------------------------------------------
//class RenderBuffer;
class RenderContext;
class Sampler;
class Shader;
class Texture;


//-----------------------------------------------------------------------------------------------
class Material
{
	friend class RenderContext;

public:
	Material( RenderContext* context );
	Material( RenderContext* context, const char* filename );
	~Material();

	void SetShader( Shader* shader )								{ m_shader = shader; }
	void SetDiffuseTexture( Texture* diffuse )						{ m_diffuseTexture = diffuse; }
	void SetNormalTexture( Texture* normal )						{ m_normalTexture = normal; }
	void SetUserTexture( uint slot, Texture* texture );
	void UpdateUBOIfDirty();
	
	//-----------------------------------------------------------------------------------------------
	void SetData( void const* data, size_t dataSize )
	{
		m_uboCPUData.resize( dataSize );
		memcpy( &m_uboCPUData[0], data, dataSize );
		m_uboIsDirty = true;
	}

	//-----------------------------------------------------------------------------------------------
	template <typename UBO_STRUCT_TYPE>
	void SetData( UBO_STRUCT_TYPE const& data )
	{
		SetData( &data, sizeof( UBO_STRUCT_TYPE ) );
	}

	//-----------------------------------------------------------------------------------------------
	template <typename UBO_STRUCT_TYPE>
	UBO_STRUCT_TYPE* GetDataAs()
	{
		m_uboIsDirty = true;
		if ( m_uboCPUData.size() == sizeof( UBO_STRUCT_TYPE ) )
		{
			return (UBO_STRUCT_TYPE*)&m_uboCPUData[0];
		}
		else
		{
			m_uboCPUData.resize( sizeof( UBO_STRUCT_TYPE ) );
			UBO_STRUCT_TYPE* retPtr = (UBO_STRUCT_TYPE*)&m_uboCPUData[0];
			new ( retPtr ) UBO_STRUCT_TYPE();

			return retPtr;
		}
	}

private:
	std::string m_filename;
	std::string m_name;

	Shader* m_shader = nullptr;

	Texture* m_diffuseTexture = nullptr;
	Texture* m_normalTexture = nullptr;

	Sampler* m_defaultSampler = nullptr;

	std::vector<Texture*> m_userTextures;
	std::vector<Sampler*> m_userSamplers;

	Rgba8 m_tint = Rgba8::WHITE;
	float m_specularFactor = 0.f;
	float m_specularPower = 32.f;

	std::vector<unsigned char> m_uboCPUData;
	RenderBuffer* m_ubo = nullptr;
	bool m_uboIsDirty = true;
};

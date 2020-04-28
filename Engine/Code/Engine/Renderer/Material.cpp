#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Sampler.hpp"


//-----------------------------------------------------------------------------------------------
Material::Material( RenderContext* context, const char* filename )
	: m_filename( filename )
{
	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filename );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		ERROR_AND_DIE( Stringf( "The shader xml file '%s' could not be opened.", filename ) );
	}

	XmlElement* materialElem = doc.RootElement();

	GUARANTEE_OR_DIE( materialElem != nullptr, Stringf( "Shader xml file '%s' doesn't have any elements", filename ) );


	m_name = ParseXmlAttribute( *materialElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Material did not have a name attribute" );

	std::string shaderPath = ParseXmlAttribute( *materialElem, "shader_path", shaderPath );
	m_shader = context->GetOrCreateShader( shaderPath.c_str() );

	const XmlElement* diffuseElem = materialElem->FirstChildElement( "diffuse" );
	std::string diffusePath = ParseXmlAttribute( *diffuseElem, "path", diffusePath );
	if ( diffusePath == "White" )
	{
		m_diffuseTexture = context->GetDefaultWhiteTexture();
	}
	else
	{
		m_diffuseTexture = context->CreateOrGetTextureFromFile( diffusePath.c_str() );
	}

	const XmlElement* normalElem = materialElem->FirstChildElement( "normal" );
	std::string normalPath = ParseXmlAttribute( *normalElem, "path", normalPath );
	if ( normalPath == "White" )
	{
		m_normalTexture = context->GetDefaultFlatTexture();
	}
	else
	{
		m_normalTexture = context->CreateOrGetTextureFromFile( normalPath.c_str() );
	}

	const XmlElement* textureElem = materialElem->FirstChildElement( "texture2d" );
	m_userTextures.resize( 8 );
	int curTextureNum = 0;
	while ( textureElem )
	{
		std::string texturePath = ParseXmlAttribute( *textureElem, "path", texturePath );
		int slot = ParseXmlAttribute( *textureElem, "slot", curTextureNum );
		++curTextureNum;

		Texture* userTexture = context->CreateOrGetTextureFromFile( texturePath .c_str() );
		if ( userTexture != nullptr )
		{
			m_userTextures[slot] = userTexture;
		}
		textureElem = textureElem->NextSiblingElement( "texture2d" );
	}

	const XmlElement* samplerElem = materialElem->FirstChildElement( "sampler" );
	m_userSamplers.resize( 8 );
	int curSamplerNum = 0;
	while ( samplerElem )
	{
		int slot = ParseXmlAttribute( *samplerElem, "slot", curSamplerNum );
		++curSamplerNum;

		eSamplerType filter = SAMPLER_POINT;
		std::string filterStr = ParseXmlAttribute( *samplerElem, "filter", "point" );
		if ( filterStr == "point" ) { filter = SAMPLER_POINT; }
		if ( filterStr == "bilinear" ) { filter = SAMPLER_BILINEAR; }

		eSamplerUVMode mode = UV_MODE_CLAMP;
		std::string modeStr = ParseXmlAttribute( *samplerElem, "mode", "clamp" );
		if ( modeStr == "clamp" ) { mode = UV_MODE_CLAMP; }
		if ( modeStr == "wrap" ) { mode = UV_MODE_WRAP; }

		Sampler* userSampler = context->GetOrCreateSampler( filter, mode );
		if ( userSampler != nullptr )
		{
			m_userSamplers[slot] = userSampler;
		}

		samplerElem = samplerElem->NextSiblingElement( "sampler" );
	}

	m_ubo = new RenderBuffer( context, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
}


//-----------------------------------------------------------------------------------------------
Material::~Material()
{
	PTR_SAFE_DELETE( m_ubo );
}


//-----------------------------------------------------------------------------------------------
void Material::SetShader( Shader* shader )
{
	m_shader = shader;
}


//-----------------------------------------------------------------------------------------------
void Material::UpdateUBOIfDirty()
{
	if ( m_uboIsDirty )
	{
		m_uboIsDirty = false;

		m_ubo->Update( (void*)&m_uboCPUData, sizeof( m_uboCPUData ), sizeof( m_uboCPUData ) );
	}
}

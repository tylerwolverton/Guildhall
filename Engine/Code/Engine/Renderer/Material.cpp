#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
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
	if ( diffuseElem == nullptr )
	{
		m_diffuseTexture = nullptr;
	}
	else
	{
		std::string diffusePath = ParseXmlAttribute( *diffuseElem, "path", diffusePath );
		if ( diffusePath == "White" )
		{
			m_diffuseTexture = context->GetDefaultWhiteTexture();
		}
		else
		{
			m_diffuseTexture = context->CreateOrGetTextureFromFile( diffusePath.c_str() );
		}
	}

	const XmlElement* normalElem = materialElem->FirstChildElement( "normal" );
	if ( normalElem == nullptr )
	{
		m_normalTexture = nullptr;
	}
	else
	{
		std::string normalPath = ParseXmlAttribute( *normalElem, "path", normalPath );
		if ( normalPath == "Flat" )
		{
			m_normalTexture = context->GetDefaultFlatTexture();
		}
		else
		{
			m_normalTexture = context->CreateOrGetTextureFromFile( normalPath.c_str() );
		}
	}

	const XmlElement* textureElem = materialElem->FirstChildElement( "texture2d" );
	m_userTextures.resize( MAX_USER_TEXTURES );
	int curTextureNum = 0;
	while ( textureElem )
	{
		if ( curTextureNum >= MAX_USER_TEXTURES )
		{
			g_devConsole->PrintString( Stringf( "Engine only supports %d textures, ignoring the rest", MAX_USER_TEXTURES ), Rgba8::YELLOW );
			break;
		}

		std::string texturePath = ParseXmlAttribute( *textureElem, "path", texturePath );
		int slot = ParseXmlAttribute( *textureElem, "slot", curTextureNum );
		if ( slot >= MAX_USER_TEXTURES )
		{
			g_devConsole->PrintString( Stringf( "Texture slot '%d' is outside range of %d - %d, skipping", slot, USER_TEXTURE_SLOT_START, USER_TEXTURE_SLOT_START + MAX_USER_TEXTURES - 1 ), Rgba8::YELLOW );
			continue;
		}
		++curTextureNum;

		Texture* userTexture = context->CreateOrGetTextureFromFile( texturePath .c_str() );
		if ( userTexture != nullptr )
		{
			m_userTextures[slot] = userTexture;
		}
		textureElem = textureElem->NextSiblingElement( "texture2d" );
	}

	const XmlElement* samplerElem = materialElem->FirstChildElement( "sampler" );
	m_userSamplers.resize( MAX_USER_TEXTURES );
	int curSamplerNum = 0;
	while ( samplerElem )
	{
		if ( curTextureNum >= MAX_USER_TEXTURES )
		{
			g_devConsole->PrintString( Stringf( "Engine only supports %d samplers, ignoring the rest", MAX_USER_TEXTURES ), Rgba8::YELLOW );
			break;
		}

		int slot = ParseXmlAttribute( *samplerElem, "slot", curSamplerNum );
		if ( slot >= MAX_USER_TEXTURES )
		{
			g_devConsole->PrintString( Stringf( "Sampler slot '%d' is outside range of 0 - %d, skipping", slot, MAX_USER_TEXTURES - 1 ), Rgba8::YELLOW );
			continue;
		}
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

	const XmlElement* specularElem = materialElem->FirstChildElement( "specular" );
	if ( specularElem != nullptr )
	{
		m_specularFactor = ParseXmlAttribute( *specularElem, "factor", m_specularFactor );
		m_specularPower = ParseXmlAttribute( *specularElem, "power", m_specularPower );
	}

	m_ubo = new RenderBuffer( context, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
}


//-----------------------------------------------------------------------------------------------
Material::Material( RenderContext* context )
{
	m_userTextures.resize( MAX_USER_TEXTURES );
	m_userSamplers.resize( MAX_USER_TEXTURES );

	m_ubo = new RenderBuffer( context, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
}


//-----------------------------------------------------------------------------------------------
Material::~Material()
{
	PTR_SAFE_DELETE( m_ubo );
}


//-----------------------------------------------------------------------------------------------
void Material::SetUserTexture( uint slot, Texture* texture )
{
	GUARANTEE_OR_DIE( slot < MAX_USER_TEXTURES, Stringf( "Must specify a user texture less than max num user textures: %d", MAX_USER_TEXTURES ) );
	
	m_userTextures[slot] = texture;
}


//-----------------------------------------------------------------------------------------------
void Material::UpdateUBOIfDirty()
{
	if ( m_uboIsDirty && m_uboCPUData.size() > 0 )
	{
		m_ubo->Update( (void*)&m_uboCPUData[0], m_uboCPUData.size(), m_uboCPUData.size() );
		
		m_uboIsDirty = false;
	}
}

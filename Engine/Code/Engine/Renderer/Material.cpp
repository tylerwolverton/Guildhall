#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
Material::Material( RenderContext* context, const char* filename, const XmlElement& materialElem )
	: m_context( context )
	, m_filename( filename )
{
	m_name = ParseXmlAttribute( materialElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Material did not have a name attribute" );

	std::string shaderPath = ParseXmlAttribute( materialElem, "shader", shaderPath );
	m_shader = m_context->GetOrCreateShader( shaderPath.c_str() );

	const XmlElement* diffuseElem = materialElem.FirstChildElement( "diffuse" );
	const XmlElement* normalElem = materialElem.FirstChildElement( "normal" );

	const XmlElement* textureElem = materialElem.FirstChildElement( "texture2d" );
	while ( textureElem )
	{

		textureElem = textureElem->NextSiblingElement( "texture2d" );
	}

	const XmlElement* samplerElem = materialElem.FirstChildElement( "sampler" );
	while ( samplerElem )
	{

		samplerElem = samplerElem->NextSiblingElement( "sampler" );
	}
}


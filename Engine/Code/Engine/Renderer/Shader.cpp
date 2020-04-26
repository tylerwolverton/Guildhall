#include "Engine/Renderer/Shader.hpp"


//-----------------------------------------------------------------------------------------------
Shader::Shader( RenderContext* context, const char* filename, const XmlElement& shaderElem )
	: m_context( context ) 
	, m_filename( filename )
{
	m_name = ParseXmlAttribute( shaderElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Shader did not have a name attribute" );

	const XmlElement* passElem = shaderElem.FirstChildElement( "Pass" );
	while ( passElem )
	{
		std::string programPath = ParseXmlAttribute( *passElem, "program", "" );
		m_program = m_context->GetOrCreateShaderProgram( programPath.c_str() );

		std::string windingStr = ParseXmlAttribute( *passElem, "front", "ccw" );
		m_state.isWindingCCW = windingStr == "ccw";

		std::string cullStr = ParseXmlAttribute( *passElem, "cull", "none" );
		if ( cullStr == "none" ) { m_state.cullMode = eCullMode::NONE; }
		if ( cullStr == "back" ) { m_state.cullMode = eCullMode::BACK; }
		if ( cullStr == "front" ) { m_state.cullMode = eCullMode::FRONT; }
		
		std::string fillStr = ParseXmlAttribute( *passElem, "fill", "solid" );
		if ( fillStr == "solid" ) { m_state.fillMode = eFillMode::SOLID; }
		if ( fillStr == "wire" ) { m_state.fillMode = eFillMode::WIREFRAME; }

		const XmlElement* blendElem = passElem->FirstChildElement( "blend" );
		if ( blendElem != nullptr )
		{
			bool blendEnabled = ParseXmlAttribute( *blendElem, "enabled", false );
			if ( blendEnabled )
			{
				std::string blendStr = ParseXmlAttribute( *blendElem, "mode", "opaque" );
				if ( blendStr == "opaque" ) { m_state.blendMode = eBlendMode::DISABLED; }
				if ( blendStr == "alpha" ) { m_state.blendMode = eBlendMode::ALPHA; }
				if ( blendStr == "additive" ) { m_state.blendMode = eBlendMode::ADDITIVE; }
			}
		}

		const XmlElement* depthElem = passElem->FirstChildElement( "depth" );
		if ( depthElem != nullptr )
		{
			bool depthEnabled = ParseXmlAttribute( *depthElem, "enabled", false );
			if ( depthEnabled )
			{
				std::string testStr = ParseXmlAttribute( *depthElem, "test", "always" );
				if ( testStr == "never" ) { m_state.depthTestCompare = eCompareFunc::COMPARISON_NEVER; }
				if ( testStr == "always" ) { m_state.depthTestCompare = eCompareFunc::COMPARISON_ALWAYS; }
				if ( testStr == "equal" ) { m_state.depthTestCompare = eCompareFunc::COMPARISON_EQUAL; }
				if ( testStr == "greater" ) { m_state.depthTestCompare = eCompareFunc::COMPARISON_GREATER; }
				if ( testStr == "greater_equal" ) { m_state.depthTestCompare = eCompareFunc::COMPARISON_GREATER_EQUAL; }
				if ( testStr == "less" ) { m_state.depthTestCompare = eCompareFunc::COMPARISON_LESS; }
				if ( testStr == "less_equal" ) { m_state.depthTestCompare = eCompareFunc::COMPARISON_LESS_EQUAL; }

				m_state.writeDpeth = ParseXmlAttribute( *depthElem, "write", false );
			}
		}

		// Only 1 pass supported for now
		passElem = nullptr;
		//passElem = passElem->NextSiblingElement();
	}
}

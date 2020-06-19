#include "Game/EntityDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
EntityDefinition::EntityDefinition( const XmlElement& entityDefElem )
{
	m_name = ParseXmlAttribute( entityDefElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Entity did not have a name attribute" );

	m_faction = ParseXmlAttribute( entityDefElem, "faction", m_faction );

	const XmlElement* sizeElement = entityDefElem.FirstChildElement( "Size" );
	if(sizeElement != nullptr)
	{
		m_physicsRadius = ParseXmlAttribute( *sizeElement, "physicsRadius", m_physicsRadius );
		m_localDrawBounds = ParseXmlAttribute( *sizeElement, "localDrawBounds", m_localDrawBounds );
	}

	const XmlElement* healthElement = entityDefElem.FirstChildElement( "Health" );
	if ( healthElement != nullptr )
	{
		m_maxHealth = ParseXmlAttribute( *healthElement, "max", m_maxHealth );
		m_startHealth = ParseXmlAttribute( *healthElement, "start", m_startHealth );
	}
}

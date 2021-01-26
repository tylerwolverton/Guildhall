#include "Game/EntityDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#include "Game/GameCommon.hpp"
#include "Game/SpriteAnimSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, EntityDefinition* > EntityDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
EntityDefinition::EntityDefinition( const XmlElement& entityDefElem )
{
	m_name = ParseXmlAttribute( entityDefElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Entity did not have a name attribute" );

	m_type = ParseXmlAttribute( entityDefElem, "type", m_type );
	m_faction = ParseXmlAttribute( entityDefElem, "faction", m_faction );

	const XmlElement* sizeElement = entityDefElem.FirstChildElement( "Size" );
	if(sizeElement != nullptr)
	{
		m_physicsRadius = ParseXmlAttribute( *sizeElement, "physicsRadius", m_physicsRadius );
		m_localDrawBounds = ParseXmlAttribute( *sizeElement, "localDrawBounds", m_localDrawBounds );
		m_drawOrder = ParseXmlAttribute( *sizeElement, "drawOrder", m_drawOrder );
	}

	const XmlElement* healthElement = entityDefElem.FirstChildElement( "Health" );
	if ( healthElement != nullptr )
	{
		m_maxHealth = ParseXmlAttribute( *healthElement, "max", m_maxHealth );
		m_startHealth = ParseXmlAttribute( *healthElement, "start", m_startHealth );
	}

	const XmlElement* spriteAnimSetElement = entityDefElem.FirstChildElement( "SpriteAnimSet" );
	if ( spriteAnimSetElement != nullptr )
	{
		m_spriteAnimSetDef = new SpriteAnimSetDefinition( *g_renderer, *spriteAnimSetElement );
	}
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{
	PTR_SAFE_DELETE( m_spriteAnimSetDef );
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* EntityDefinition::GetSpriteAnimDef( const std::string& animName )
{
	if ( m_spriteAnimSetDef == nullptr )
	{
		return nullptr;
	}

	std::map< std::string, SpriteAnimDefinition* >::const_iterator  mapIter = m_spriteAnimSetDef->m_spriteAnimDefMapByName.find( animName );

	if ( mapIter == m_spriteAnimSetDef->m_spriteAnimDefMapByName.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition* EntityDefinition::GetEntityDefinition( std::string entityName )
{
	std::map< std::string, EntityDefinition* >::const_iterator  mapIter = EntityDefinition::s_definitions.find( entityName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


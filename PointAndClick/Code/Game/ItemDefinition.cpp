#include "Game/ItemDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteAnimSetDefinition.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, ItemDefinition* > ItemDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
ItemDefinition::ItemDefinition( const XmlElement& itemDefElem )
	: EntityDefinition( itemDefElem )
{
	const XmlElement* spriteAnimSetElement = itemDefElem.FirstChildElement( "SpriteAnimSet" );
	if ( spriteAnimSetElement != nullptr )
	{
		m_spriteAnimSetDef = new SpriteAnimSetDefinition( *g_renderer, *spriteAnimSetElement );
	}
}


//-----------------------------------------------------------------------------------------------
ItemDefinition::~ItemDefinition()
{
	delete m_spriteAnimSetDef;
	m_spriteAnimSetDef = nullptr;
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* ItemDefinition::GetSpriteAnimDef( const std::string& animName )
{
	std::map< std::string, SpriteAnimDefinition* >::const_iterator  mapIter = m_spriteAnimSetDef->m_spriteAnimDefMapByName.find( animName );

	if ( mapIter == m_spriteAnimSetDef->m_spriteAnimDefMapByName.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ItemDefinition* ItemDefinition::GetItemDefinition( const std::string& itemName )
{
	std::map< std::string, ItemDefinition* >::const_iterator  mapIter = ItemDefinition::s_definitions.find( itemName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}

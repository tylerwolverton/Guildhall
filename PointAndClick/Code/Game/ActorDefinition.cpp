#include "Game/ActorDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteAnimSetDefinition.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, ActorDefinition* > ActorDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
ActorDefinition* ActorDefinition::GetActorDefinition( std::string actorName )
{
	std::map< std::string, ActorDefinition* >::const_iterator  mapIter = ActorDefinition::s_definitions.find( actorName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ActorDefinition::ActorDefinition( const XmlElement& actorDefElem )
	: EntityDefinition( actorDefElem )
{
	const XmlElement* movementElement = actorDefElem.FirstChildElement( "Movement" );
	if ( movementElement != nullptr )
	{
		const XmlElement* walkElement = movementElement->FirstChildElement( "Walk" );
		if ( walkElement != nullptr )
		{
			m_walkSpeed = ParseXmlAttribute( *walkElement, "speed", m_walkSpeed );
		}
	}
}


//-----------------------------------------------------------------------------------------------
ActorDefinition::~ActorDefinition()
{
	delete m_spriteAnimSetDef;
	m_spriteAnimSetDef = nullptr;
}

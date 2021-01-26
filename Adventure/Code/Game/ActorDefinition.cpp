#include "Game/ActorDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#include "Game/GameCommon.hpp"
#include "Game/SpriteAnimSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, ActorDefinition* > ActorDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* ActorDefinition::GetSpriteAnimDef( std::string animName )
{
	return SpriteAnimSetDefinition::GetSpriteAnimDefinitionByName( animName );
}


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
			m_canWalk = true;
			m_walkSpeed = ParseXmlAttribute( *walkElement, "speed", m_walkSpeed );
		}

		const XmlElement* flyElement = movementElement->FirstChildElement( "Fly" );
		if ( flyElement != nullptr )
		{
			m_canFly = true;
			m_flySpeed = ParseXmlAttribute( *flyElement, "speed", m_flySpeed );
		}

		const XmlElement* swimElement = movementElement->FirstChildElement( "Swim" );
		if ( swimElement != nullptr )
		{
			m_canSwim = true;
			m_swimSpeed = ParseXmlAttribute( *swimElement, "speed", m_swimSpeed );
		}
	}

	const XmlElement* spriteAnimSetElement = actorDefElem.FirstChildElement( "SpriteAnimSet" );
	if ( spriteAnimSetElement != nullptr )
	{
		m_spriteAnimSetDef = new SpriteAnimSetDefinition( *g_renderer, *spriteAnimSetElement );
	}
}


//-----------------------------------------------------------------------------------------------
ActorDefinition::~ActorDefinition()
{
	delete m_spriteAnimSetDef;
	m_spriteAnimSetDef = nullptr;
}

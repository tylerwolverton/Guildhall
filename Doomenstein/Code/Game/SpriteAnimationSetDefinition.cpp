#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition::SpriteAnimationSetDefinition( SpriteSheet* spriteSheet, const XmlElement& spriteAnimSetDefElem )
	: m_spriteSheet( spriteSheet )
{
	m_name = spriteAnimSetDefElem.Name();

	if ( !spriteAnimSetDefElem.Attribute( "front" ) )
	{
		Ints spriteIndexes;
		spriteIndexes = ParseXmlAttribute( spriteAnimSetDefElem, "front", spriteIndexes );
		if ( spriteIndexes.size() == 0 )
		{
			g_devConsole->PrintError( Stringf( "Animation '%s' front has no sprite indexes defined.", m_name.c_str() ) );
		}
		else
		{
			m_frontAnimDef = new SpriteAnimDefinition( *m_spriteSheet, spriteIndexes, 1.f );
		}
	}	
}


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition::~SpriteAnimationSetDefinition()
{
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* SpriteAnimationSetDefinition::GetSpriteAnimationDefForDirection( const Transform& entityTransform, const Transform& cameraTransform )
{
	return m_frontAnimDef;
}

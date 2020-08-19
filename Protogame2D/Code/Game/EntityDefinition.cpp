#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, EntityDefinition* > EntityDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition* EntityDefinition::GetSpriteAnimSetDef( const std::string& animSetName ) const
{
	auto mapIter = m_spriteAnimSetDefs.find( animSetName );

	if ( mapIter == m_spriteAnimSetDefs.cend() )
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


//-----------------------------------------------------------------------------------------------
EntityDefinition::EntityDefinition( const XmlElement& entityDefElem )
{
	m_name = ParseXmlAttribute( entityDefElem, "name", "" );
	if ( m_name == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: EntityType is missing a name attribute" );
		return;
	}

	std::string typeStr = entityDefElem.Name();
	if ( typeStr == "Entity" )
	{
		m_type = eEntityType::ENTITY;
	}
	else if ( typeStr == "Actor" )
	{
		m_type = eEntityType::ACTOR;
	}
	else if ( typeStr == "Projectile" )
	{
		m_type = eEntityType::PROJECTILE;
	}
	else if ( typeStr == "Portal" )
	{
		m_type = eEntityType::PORTAL;
	}
	else
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unsupported entity type seen, '%s'", typeStr.c_str() ) );
		return;
	}


	const XmlElement* physicsElem = entityDefElem.FirstChildElement( "Physics" );
	if ( physicsElem != nullptr )
	{
		m_physicsRadius = ParseXmlAttribute( *physicsElem, "radius", m_physicsRadius );
		m_walkSpeed = ParseXmlAttribute( *physicsElem, "walkSpeed", m_walkSpeed );
	}

	const XmlElement* appearanceElem = entityDefElem.FirstChildElement( "Appearance" );
	if ( appearanceElem != nullptr )
	{
		m_localDrawBounds = ParseXmlAttribute( *appearanceElem, "localDrawBounds", m_localDrawBounds );

		std::string spriteSheetPath = ParseXmlAttribute( *appearanceElem, "spriteSheet", "" );
		if ( spriteSheetPath == "" )
		{
			g_devConsole->PrintError( Stringf( "Actor '%s' has an appearance node but no spriteSheet attribute", m_name.c_str() ) );
			return;
		}
		
		SpriteSheet* spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
		if ( spriteSheet == nullptr )
		{
			IntVec2 layout = ParseXmlAttribute( *appearanceElem, "layout", IntVec2( -1, -1 ) );
			if ( layout == IntVec2( -1, -1 ) )
			{
				g_devConsole->PrintError( Stringf( "Actor '%s' has an appearance node but no layout attribute", m_name.c_str() ) );
				return;
			}

			Texture* texture = g_renderer->CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
			if ( texture == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Actor '%s' couldn't load texture '%s'", m_name.c_str(), spriteSheetPath.c_str() ) );
				return;
			}

			spriteSheet = SpriteSheet::CreateAndRegister( *texture, layout );
		}

		const XmlElement* animationSetElem = appearanceElem->FirstChildElement();
		while ( animationSetElem != nullptr )
		{
			m_spriteAnimSetDefs[animationSetElem->Name()] = new SpriteAnimationSetDefinition( spriteSheet, *animationSetElem );

			animationSetElem = animationSetElem->NextSiblingElement();
		}
	}

	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{
	PTR_MAP_SAFE_DELETE( m_spriteAnimSetDefs );
}


//-----------------------------------------------------------------------------------------------
std::string GetEntityTypeAsString( eEntityType entityType )
{
	switch ( entityType )
	{
		case eEntityType::ACTOR: return "Actor";
		case eEntityType::PROJECTILE: return "Projectile";
		case eEntityType::PORTAL: return "Portal";
		case eEntityType::ENTITY: return "Entity";
		default: return "Unknown";
	}
}

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
	
	m_nameEnum = GetEnumFromEntityName( m_name );

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
	if( physicsElem != nullptr )
	{
		m_physicsRadius = ParseXmlAttribute( *physicsElem, "radius", m_physicsRadius );
		m_height = ParseXmlAttribute( *physicsElem, "height", m_height );
		m_eyeHeight = ParseXmlAttribute( *physicsElem, "eyeHeight", m_eyeHeight );
		m_walkSpeed = ParseXmlAttribute( *physicsElem, "walkSpeed", m_walkSpeed );
	}

	const XmlElement* appearanceElem = entityDefElem.FirstChildElement( "Appearance" );
	if ( appearanceElem != nullptr )
	{
		m_visualSize = ParseXmlAttribute( *appearanceElem, "size", m_visualSize );

		std::string spriteSheetPath = ParseXmlAttribute( *appearanceElem, "spriteSheet", "" );
		if ( spriteSheetPath == "" )
		{
			g_devConsole->PrintError( Stringf( "Actor '%s' has an appearance node but no spriteSheet attribute", m_name.c_str() ) );
			return;
		}

		std::string billboardStyleStr = ParseXmlAttribute( *appearanceElem, "billboard", "" );
		if ( billboardStyleStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Actor '%s' has an appearance node but no billboard attribute", m_name.c_str() ) );
			return;
		}
		m_billboardStyle = GetBillboardStyleFromString( billboardStyleStr );

		SpriteSheet* spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
		if ( spriteSheet == nullptr )
		{
			IntVec2 layout = ParseXmlAttribute( *appearanceElem, "layout", IntVec2( -1,-1 ) );
			if ( layout == IntVec2( -1,-1 ) )
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
		case eEntityType::ACTOR:		return "Actor";
		case eEntityType::PROJECTILE:	return "Projectile";
		case eEntityType::PORTAL:		return "Portal";
		case eEntityType::ENTITY:		return "Entity";
		default:						return "Unknown";
	}
}


//-----------------------------------------------------------------------------------------------
eEntityName GetEnumFromEntityName( const std::string& name )
{
	if ( name == "Player" )					return eEntityName::PLAYER;
	else if ( name == "Pinky" )				return eEntityName::PINKY;
	else if ( name == "Marine" )			return eEntityName::MARINE;
	else if ( name == "Energy Teleporter" ) return eEntityName::ENERGY_TELEPORTER;
	else if ( name == "Imp" )				return eEntityName::IMP;
	else if ( name == "Lamp" )				return eEntityName::LAMP;
	else if ( name == "Plasma Bolt" )		return eEntityName::PLASMA_BOLT;

	return eEntityName::NONE;
}


//-----------------------------------------------------------------------------------------------
std::string GetEntityNameEnumAsString( eEntityName entityNameEnum )
{
	switch ( entityNameEnum )
	{
		case eEntityName::PLAYER:				return "Player";
		case eEntityName::PINKY:				return "Pinky";
		case eEntityName::MARINE:				return "Marine";
		case eEntityName::ENERGY_TELEPORTER:	return "Energy Teleporter";
		case eEntityName::IMP:					return "Imp";
		case eEntityName::LAMP:					return "Lamp";
		case eEntityName::PLASMA_BOLT:			return "Plasma Bolt";
		default:								return "Unknown";
	}
}

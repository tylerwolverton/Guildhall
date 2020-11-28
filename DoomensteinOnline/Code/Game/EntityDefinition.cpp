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
	
	m_type = GetEnumFromEntityType( m_name );

	std::string classStr = entityDefElem.Name();
	if ( classStr == "Entity" )
	{
		m_class = eEntityClass::ENTITY;
	}
	else if ( classStr == "Actor" )
	{
		m_class = eEntityClass::ACTOR;
	}
	else if ( classStr == "Projectile" )
	{
		m_class = eEntityClass::PROJECTILE;
	}
	else if ( classStr == "Portal" )
	{
		m_class = eEntityClass::PORTAL;
	}
	else
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unsupported entity class type seen, '%s'", classStr.c_str() ) );
		return;
	}

	// Physics
	const XmlElement* physicsElem = entityDefElem.FirstChildElement( "Physics" );
	if( physicsElem != nullptr )
	{
		m_physicsRadius = ParseXmlAttribute( *physicsElem, "radius", m_physicsRadius );
		m_height = ParseXmlAttribute( *physicsElem, "height", m_height );
		m_eyeHeight = ParseXmlAttribute( *physicsElem, "eyeHeight", m_eyeHeight );
		m_walkSpeed = ParseXmlAttribute( *physicsElem, "walkSpeed", m_walkSpeed );
	}

	// Appearance
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
std::string GetEntityClassAsString( eEntityClass entityClass )
{
	switch ( entityClass )
	{
		case eEntityClass::ACTOR:		return "Actor";
		case eEntityClass::PROJECTILE:	return "Projectile";
		case eEntityClass::PORTAL:		return "Portal";
		case eEntityClass::ENTITY:		return "Entity";
		default:						return "Unknown";
	}
}


//-----------------------------------------------------------------------------------------------
eEntityType GetEnumFromEntityType( const std::string& name )
{
	if ( name == "Player" )					return eEntityType::PLAYER;
	else if ( name == "Pinky" )				return eEntityType::PINKY;
	else if ( name == "Marine" )			return eEntityType::MARINE;
	else if ( name == "Energy Teleporter" ) return eEntityType::ENERGY_TELEPORTER;
	else if ( name == "Imp" )				return eEntityType::IMP;
	else if ( name == "Lamp" )				return eEntityType::LAMP;
	else if ( name == "Plasma Bolt" )		return eEntityType::PLASMA_BOLT;

	return eEntityType::NONE;
}


//-----------------------------------------------------------------------------------------------
std::string GetEntityTypeAsString( eEntityType type )
{
	switch ( type )
	{
		case eEntityType::PLAYER:				return "Player";
		case eEntityType::PINKY:				return "Pinky";
		case eEntityType::MARINE:				return "Marine";
		case eEntityType::ENERGY_TELEPORTER:	return "Energy Teleporter";
		case eEntityType::IMP:					return "Imp";
		case eEntityType::LAMP:					return "Lamp";
		case eEntityType::PLASMA_BOLT:			return "Plasma Bolt";
		default:								return "Unknown";
	}
}

#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/PhysicsConfig.hpp"


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
	: ZephyrEntityDefinition( entityDefElem )
{
	m_type = ParseXmlAttribute( entityDefElem, "name", "" );
	if ( m_type == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: EntityType is missing a name attribute" );
		return;
	}
	
	std::string typeStr = entityDefElem.Name();
	if ( typeStr == "Entity" )
	{
		m_class = eEntityClass::ENTITY;
	}
	else if ( typeStr == "Actor" )
	{
		m_class = eEntityClass::ACTOR;
	}
	else if ( typeStr == "Projectile" )
	{
		m_class = eEntityClass::PROJECTILE;
	}
	else if ( typeStr == "Portal" )
	{
		m_class = eEntityClass::PORTAL;
	}
	else
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unsupported entity type seen, '%s'", typeStr.c_str() ) );
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
		
		m_initialCollisionLayer = ParseXmlAttribute( *physicsElem, "collisionLayer", m_initialCollisionLayer );
		if ( !g_physicsConfig->IsLayerDefined( m_initialCollisionLayer ) )
		{
			g_devConsole->PrintError( Stringf( "Layer '%s' has not been defined in PhysicsConfig.xml", m_initialCollisionLayer.c_str() ) );
			m_initialCollisionLayer = "";
		}

		m_initialCanBePushed = ParseXmlAttribute( *physicsElem, "canBePushed", m_initialCanBePushed );
		m_initialCanPush = ParseXmlAttribute( *physicsElem, "canPush", m_initialCanPush );
	}

	// Appearance
	const XmlElement* appearanceElem = entityDefElem.FirstChildElement( "Appearance" );
	if ( appearanceElem != nullptr )
	{
		m_visualSize = ParseXmlAttribute( *appearanceElem, "size", m_visualSize );

		std::string spriteSheetPath = ParseXmlAttribute( *appearanceElem, "spriteSheet", "" );
		if ( spriteSheetPath == "" )
		{
			g_devConsole->PrintError( Stringf( "Entity '%s' has an appearance node but no spriteSheet attribute", m_type.c_str() ) );
			return;
		}

		std::string spriteMaterialPath = ParseXmlAttribute( *appearanceElem, "spriteMaterial", "" );
		if ( spriteMaterialPath != "" )
		{
			m_spriteMaterial = new Material( g_renderer, spriteMaterialPath.c_str() );
		}

		std::string billboardStyleStr = ParseXmlAttribute( *appearanceElem, "billboard", "" );
		if ( billboardStyleStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Entity '%s' has an appearance node but no billboard attribute", m_type.c_str() ) );
			return;
		}
		m_billboardStyle = GetBillboardStyleFromString( billboardStyleStr );

		SpriteSheet* spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
		if ( spriteSheet == nullptr )
		{
			IntVec2 layout = ParseXmlAttribute( *appearanceElem, "layout", IntVec2( -1,-1 ) );
			if ( layout == IntVec2( -1,-1 ) )
			{
				g_devConsole->PrintError( Stringf( "Entity '%s' has an appearance node but no layout attribute", m_type.c_str() ) );
				return;
			}

			Texture* texture = g_renderer->CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
			if ( texture == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Entity '%s' couldn't load texture '%s'", m_type.c_str(), spriteSheetPath.c_str() ) );
				return;
			}

			spriteSheet = SpriteSheet::CreateAndRegister( *texture, layout );
		}

		const XmlElement* animationSetElem = appearanceElem->FirstChildElement();
		while ( animationSetElem != nullptr )
		{
			m_spriteAnimSetDefs[animationSetElem->Name()] = new SpriteAnimationSetDefinition( spriteSheet, m_spriteMaterial, *animationSetElem );

			animationSetElem = animationSetElem->NextSiblingElement();
		}
	}

	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{
	PTR_MAP_SAFE_DELETE( m_spriteAnimSetDefs );
	PTR_SAFE_DELETE( m_spriteMaterial );
}


//-----------------------------------------------------------------------------------------------
std::string GetEntityClassAsString( eEntityClass entityType )
{
	switch ( entityType )
	{
		case eEntityClass::ACTOR: return "Actor";
		case eEntityClass::PROJECTILE: return "Projectile";
		case eEntityClass::PORTAL: return "Portal";
		case eEntityClass::ENTITY: return "Entity";
		default: return "Unknown";
	}
}

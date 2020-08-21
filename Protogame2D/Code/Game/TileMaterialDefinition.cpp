#include "Game/TileMaterialDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, TileMaterialDefinition* > TileMaterialDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
TileMaterialDefinition* TileMaterialDefinition::GetTileMaterialDefinition( std::string tileMaterialName )
{
	std::map< std::string, TileMaterialDefinition* >::const_iterator  mapIter = TileMaterialDefinition::s_definitions.find( tileMaterialName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
TileMaterialDefinition::TileMaterialDefinition( const XmlElement& tileMatDefElem, SpriteSheet* spriteSheet )
	: m_spriteSheet( spriteSheet )
{
	m_name = ParseXmlAttribute( tileMatDefElem, "name", m_name.c_str() );
	if ( m_name == "" )
	{
		g_devConsole->PrintError( "Material type is missing a name" );
		return;
	}

	m_spriteCoords = ParseXmlAttribute( tileMatDefElem, "spriteCoords", m_spriteCoords );
	if ( m_spriteCoords == IntVec2( -1, -1 ) )
	{
		g_devConsole->PrintError( Stringf( "Material type '%s' is missing a spriteCoords attribute", m_name.c_str() ) );
		return;
	}

	IntVec2 spriteSheetDimensions = m_spriteSheet->GetDimensions();
	if ( m_spriteCoords.x < 0
		 || m_spriteCoords.x > spriteSheetDimensions.x )
	{
		g_devConsole->PrintError( Stringf( "Material type '%s' specifies an out of bounds x sprite coordinate '%d' ", m_name.c_str(), m_spriteCoords.x ) );
		return;
	}

	if ( m_spriteCoords.y < 0
		 || m_spriteCoords.y > spriteSheetDimensions.y )
	{
		g_devConsole->PrintError( Stringf( "Material type '%s' specifies an out of bounds y sprite coordinate '%d' ", m_name.c_str(), m_spriteCoords.y ) );
		return;
	}
	
	IntVec2 spriteCoords; 
	m_spriteCoords = ParseXmlAttribute( tileMatDefElem, "spriteCoords", m_spriteCoords );

	Vec2 mins, maxs;
	m_spriteSheet->GetSpriteUVs( mins, maxs, spriteCoords );
	m_uvCoords.mins = mins;
	m_uvCoords.maxs = maxs;

	std::string spriteTintStr = ParseXmlAttribute( tileMatDefElem, "spriteTint", "255,255,255,255" );
	m_spriteTint.SetFromText( spriteTintStr.c_str() );

	m_isValid = true;
}

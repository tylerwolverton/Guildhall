#include "Game/TileDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, TileDefinition* > TileDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
TileDefinition* TileDefinition::GetTileDefinition( std::string tileName )
{
	std::map< std::string, TileDefinition* >::const_iterator  mapIter = TileDefinition::s_definitions.find( tileName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
TileDefinition* TileDefinition::GetTileDefinitionFromImageTexelColor( const Rgba8& imageTexelColor )
{
	for ( const std::pair< std::string, TileDefinition* >& tileElem : s_definitions )
	{
		if ( tileElem.second->m_tileImageColor.IsRGBEqual( imageTexelColor ) )
		{
			return tileElem.second;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
TileDefinition::TileDefinition( const XmlElement& tileDefElem )
{
	m_name = ParseXmlAttribute( tileDefElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Tile did not have a name attribute" );

	m_tileImageColor = ParseXmlAttribute( tileDefElem, "tileImageColor", m_tileImageColor );

	IntVec2 spriteCoords; 
	std::string spriteCoordsStr = ParseXmlAttribute( tileDefElem, "spriteCoords", "0,0" );
	spriteCoords.SetFromText( spriteCoordsStr.c_str() );
	
	std::string spriteTintStr = ParseXmlAttribute( tileDefElem, "spriteTint", "255,255,255,255" );
	m_spriteTint.SetFromText( spriteTintStr.c_str() );
}

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
TileDefinition::TileDefinition( const XmlElement& tileDefElem, const std::string& defaultMaterialName )
{
	m_name = ParseXmlAttribute( tileDefElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Tile did not have a name attribute" );

	std::string materialName = ParseXmlAttribute( tileDefElem, "materialType", defaultMaterialName );
	m_matDef = TileMaterialDefinition::GetTileMaterialDefinition( materialName );

	m_isSolid = ParseXmlAttribute( tileDefElem, "isSolid", m_isSolid );

	m_isValid = true;
}

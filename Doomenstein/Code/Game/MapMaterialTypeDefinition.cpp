#include "Game/MapMaterialTypeDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, MapMaterialTypeDefinition* > MapMaterialTypeDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
MapMaterialTypeDefinition* MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( std::string mapMaterialTypeName )
{
	std::map< std::string, MapMaterialTypeDefinition* >::const_iterator  mapIter = MapMaterialTypeDefinition::s_definitions.find( mapMaterialTypeName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
MapMaterialTypeDefinition::MapMaterialTypeDefinition( const XmlElement& mapMaterialTypeDefElem )
{
	m_name = ParseXmlAttribute( mapMaterialTypeDefElem, "name", m_name );
	// TODO: Check name exists
	std::string sheetStr = ParseXmlAttribute( mapMaterialTypeDefElem, "sheet", "" );
	m_sheet = SpriteSheet::GetSpriteSheet( sheetStr );
	// TODO: Check sheet exists
	m_spriteCoords = ParseXmlAttribute( mapMaterialTypeDefElem, "spriteCoords", m_spriteCoords );
}


//-----------------------------------------------------------------------------------------------
MapMaterialTypeDefinition::~MapMaterialTypeDefinition()
{

}

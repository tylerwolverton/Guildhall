#include "Game/MapMaterialTypeDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
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
	m_name = ParseXmlAttribute( mapMaterialTypeDefElem, "name", m_name.c_str() );
	if ( m_name == "" )
	{
		g_devConsole->PrintError( "Material type is missing a name" );
		return;
	}

	std::string sheetStr = ParseXmlAttribute( mapMaterialTypeDefElem, "sheet", "" );
	if ( sheetStr == "" )
	{
		g_devConsole->PrintError( Stringf( "Material type '%s' is missing a sheet attribute", m_name.c_str() ) );
		return;
	}

	m_sheet = SpriteSheet::GetSpriteSheet( sheetStr );
	if ( m_sheet == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Material type '%s' references a sprite sheet '%s' that isn't defined", m_name.c_str(), sheetStr.c_str() ) );
		return;
	}

	m_spriteCoords = ParseXmlAttribute( mapMaterialTypeDefElem, "spriteCoords", m_spriteCoords );
	if ( m_spriteCoords == IntVec2( -1, -1 ) )
	{
		g_devConsole->PrintError( Stringf( "Material type '%s' is missing a spriteCoords attribute", m_name.c_str() ) );
		return;
	}

	IntVec2 spriteSheetDimensions = m_sheet->GetDimensions();
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

	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
MapMaterialTypeDefinition::~MapMaterialTypeDefinition()
{

}

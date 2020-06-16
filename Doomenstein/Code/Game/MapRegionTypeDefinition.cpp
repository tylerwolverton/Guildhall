#include "Game/MapRegionTypeDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/MapMaterialTypeDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, MapRegionTypeDefinition* > MapRegionTypeDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
MapRegionTypeDefinition* MapRegionTypeDefinition::GetMapRegionTypeDefinition( std::string mapRegionTypeName )
{
	std::map< std::string, MapRegionTypeDefinition* >::const_iterator  mapIter = MapRegionTypeDefinition::s_definitions.find( mapRegionTypeName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
MapRegionTypeDefinition::MapRegionTypeDefinition( const XmlElement& mapRegionTypeDefElem, const std::string& defaultMaterialName )
{
	m_name = ParseXmlAttribute( mapRegionTypeDefElem, "name", m_name );
	if ( m_name == "" )
	{
		g_devConsole->PrintError( "Region type is missing a name" );
		return;
	}

	std::string isSolidStr = ParseXmlAttribute( mapRegionTypeDefElem, "isSolid", "" );
	if ( isSolidStr == "" )
	{
		g_devConsole->PrintError( Stringf( "Region type '%s' is missing isSolid attribute", m_name.c_str() ) );
		return;
	}

	m_sideMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( defaultMaterialName );
	m_floorMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( defaultMaterialName );
	m_ceilingMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( defaultMaterialName );

	m_isSolid = ParseXmlAttribute( mapRegionTypeDefElem, "isSolid", m_isSolid );

	if ( m_isSolid )
	{
		// Parse side
		const XmlElement* sideElem = mapRegionTypeDefElem.FirstChildElement( "Side" );
		if ( sideElem == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' is solid but has no Side node defined", m_name.c_str() ) );
			return;
		}

		std::string sideMaterialStr = ParseXmlAttribute( *sideElem, "material", "" );
		if ( sideMaterialStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' does not have a material defined", m_name.c_str() ) );
			sideMaterialStr = defaultMaterialName;
		}

		m_sideMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( sideMaterialStr );
		if ( m_sideMaterial == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' referenced undefined material type '%s'", m_name.c_str(), sideMaterialStr.c_str() ) );
			m_sideMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( defaultMaterialName );
		}
	}
	else
	{
		// Parse floor
		const XmlElement* floorElem = mapRegionTypeDefElem.FirstChildElement( "Floor" );
		if ( floorElem == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' is not solid but has no Floor node defined", m_name.c_str() ) );
			return;
		}
		
		std::string floorMaterialStr = ParseXmlAttribute( *floorElem, "material", "" );
		if ( floorMaterialStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' does not have a material defined", m_name.c_str() ) );
			floorMaterialStr = defaultMaterialName;
		}

		m_floorMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( floorMaterialStr );
		if ( m_floorMaterial == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' referenced undefined material type '%s'", m_name.c_str(), floorMaterialStr.c_str() ) );
			m_floorMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( defaultMaterialName );
		}

		// Parse ceiling
		const XmlElement* ceilingElem = mapRegionTypeDefElem.FirstChildElement( "Ceiling" );
		if ( ceilingElem == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' is not solid but has no Ceiling node defined", m_name.c_str() ) );
			return;
		}

		std::string ceilingMaterialStr = ParseXmlAttribute( *ceilingElem, "material", std::string( "InvalidRegion" ) );
		if ( ceilingMaterialStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' does not have a material defined", m_name.c_str() ) );
			ceilingMaterialStr = defaultMaterialName;
		}

		m_ceilingMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( ceilingMaterialStr );
		if ( m_ceilingMaterial == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Region type '%s' referenced undefined material type '%s'", m_name.c_str(), ceilingMaterialStr.c_str() ) );
			m_ceilingMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( defaultMaterialName );
		}
	}
	
	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
MapRegionTypeDefinition::MapRegionTypeDefinition()
{

}


//-----------------------------------------------------------------------------------------------
MapRegionTypeDefinition::~MapRegionTypeDefinition()
{

}

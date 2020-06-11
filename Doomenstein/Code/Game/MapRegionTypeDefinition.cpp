#include "Game/MapRegionTypeDefinition.hpp"
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
MapRegionTypeDefinition::MapRegionTypeDefinition( const XmlElement& mapRegionTypeDefElem )
{
	m_name = ParseXmlAttribute( mapRegionTypeDefElem, "name", m_name );
	// TODO: Check name exists
	m_isSolid = ParseXmlAttribute( mapRegionTypeDefElem, "isSolid", m_isSolid );
	// TODO: Check isSolid exists

	if ( m_isSolid )
	{
		// Parse side
		const XmlElement* sideElem = mapRegionTypeDefElem.FirstChildElement( "Side" );
		std::string sideMaterialStr = ParseXmlAttribute( *sideElem, "material", std::string("InvalidRegion") );

		m_sideMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( sideMaterialStr );
	}
	else
	{
		const XmlElement* floorElem = mapRegionTypeDefElem.FirstChildElement( "Floor" );
		std::string floorMaterialStr = ParseXmlAttribute( *floorElem, "material", std::string("InvalidRegion") );
		m_floorMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( floorMaterialStr );

		const XmlElement* ceilingElem = mapRegionTypeDefElem.FirstChildElement( "Ceiling" );
		std::string ceilingMaterialStr = ParseXmlAttribute( *ceilingElem, "material", std::string( "InvalidRegion" ) );
		m_ceilingMaterial = MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( ceilingMaterialStr );
	}
	
}


//-----------------------------------------------------------------------------------------------
MapRegionTypeDefinition::MapRegionTypeDefinition()
{

}


//-----------------------------------------------------------------------------------------------
MapRegionTypeDefinition::~MapRegionTypeDefinition()
{

}

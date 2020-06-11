#include "Game/MapRegionTypeDefinition.hpp"


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
	}
	else
	{
		const XmlElement* floorElem = mapRegionTypeDefElem.FirstChildElement( "Floor" );
		const XmlElement* ceilingElem = mapRegionTypeDefElem.FirstChildElement( "Ceiling" );
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

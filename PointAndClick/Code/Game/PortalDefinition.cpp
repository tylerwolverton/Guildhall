#include "Game/PortalDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, PortalDefinition* > PortalDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
PortalDefinition::PortalDefinition( const XmlElement& portalDefElem )
	: EntityDefinition( portalDefElem )
{
	const XmlElement* destinationElem = portalDefElem.FirstChildElement( "Destination" );
	if ( destinationElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Portal '%s' must have a destination", m_name.c_str() ) );
		return;
	}

	m_destinationMapName = ParseXmlAttribute( *destinationElem, "mapName", "" );

	const XmlElement* conditionsElem = portalDefElem.FirstChildElement( "Conditions" );
	if ( conditionsElem != nullptr )
	{
		const XmlElement* conditionElem = conditionsElem->FirstChildElement( "Condition" );
		while ( conditionElem != nullptr )
		{
			/*std::string conditionType = ParseXmlAttribute( *destinationElem, "type", "" );
			if ( conditionType == "doorIsOpen" )
			{
				std::string doorName = ParseXmlAttribute( *destinationElem, "door", "" );

			}*/


			conditionElem = conditionElem->NextSiblingElement();
		}
	}
}


//-----------------------------------------------------------------------------------------------
PortalDefinition::~PortalDefinition()
{

}


//-----------------------------------------------------------------------------------------------
PortalDefinition* PortalDefinition::GetPortalDefinition( const std::string& portalName )
{
	std::map< std::string, PortalDefinition* >::const_iterator  mapIter = PortalDefinition::s_definitions.find( portalName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}

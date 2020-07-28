#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/EntityDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class Item;


//-----------------------------------------------------------------------------------------------
class PortalDefinition : public EntityDefinition
{
	friend class Portal;

public:
	explicit PortalDefinition( const XmlElement& portalDefElem );
	~PortalDefinition();

	std::string GetName() { return m_name; }
	std::string GetDestinationMapName() { return m_destinationMapName; }

	static PortalDefinition* GetPortalDefinition( const std::string& itemName );

public:
	static std::map< std::string, PortalDefinition* > s_definitions;

private:
	std::string m_destinationMapName;

	//Item* m_linkedDoor = nullptr;
};

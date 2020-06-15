#pragma once
#include "Engine/Core/XmlUtils.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class MapMaterialTypeDefinition;


//-----------------------------------------------------------------------------------------------
class MapRegionTypeDefinition
{
	friend class MapDefinition;

public:
	MapRegionTypeDefinition();
	explicit MapRegionTypeDefinition( const XmlElement& mapRegionTypeDefElem, const std::string& defaultMaterialName );
	~MapRegionTypeDefinition();

	bool IsValid() const									{ return m_isValid; }
	std::string GetName() const								{ return m_name; }
	bool IsSolid() const									{ return m_isSolid; }

	MapMaterialTypeDefinition* GetSideMaterial() const		{ return m_sideMaterial; }
	MapMaterialTypeDefinition* GetFloorMaterial() const		{ return m_floorMaterial; }
	MapMaterialTypeDefinition* GetCeilingMaterial() const	{ return m_ceilingMaterial; }

	static MapRegionTypeDefinition* GetMapRegionTypeDefinition( std::string mapRegionTypeName );

public:
	static std::map< std::string, MapRegionTypeDefinition* > s_definitions;

private:
	bool m_isValid = false;
	std::string m_name;
	std::string m_defaultRegionTypeStr;
	bool m_isSolid = false;
	MapMaterialTypeDefinition* m_sideMaterial = nullptr;
	MapMaterialTypeDefinition* m_floorMaterial = nullptr;
	MapMaterialTypeDefinition* m_ceilingMaterial = nullptr;
};

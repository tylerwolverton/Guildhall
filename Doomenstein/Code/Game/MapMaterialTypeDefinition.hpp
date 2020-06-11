#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;


//-----------------------------------------------------------------------------------------------
class MapMaterialTypeDefinition
{
public:
	explicit MapMaterialTypeDefinition( const XmlElement& mapRegionTypeDefElem );
	~MapMaterialTypeDefinition();

	std::string GetName() const							{ return m_name; }
	SpriteSheet* GetSpriteSheet() const					{ return m_sheet; }
	IntVec2 GetSpriteCoords() const						{ return m_spriteCoords; }

	static MapMaterialTypeDefinition* GetMapMaterialTypeDefinition( std::string mapMaterialTypeName );

public:
	static std::map< std::string, MapMaterialTypeDefinition* > s_definitions;

private:
	std::string m_name;
	SpriteSheet* m_sheet;
	IntVec2 m_spriteCoords;
};

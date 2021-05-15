#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"

#include "Game/TileMaterialDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class TileDefinition
{
	friend class Tile;

public:
	explicit TileDefinition( const XmlElement& tileDefElem, const std::string& defaultMaterialName );

	bool IsValid() const											{ return m_isValid; }
	std::string GetName() const										{ return m_name; }
	const AABB2 GetUVCoords() const									{ return m_matDef->GetUVCoords(); }
	const Rgba8 GetSpriteTint()	const								{ return m_matDef->GetSpriteTint(); }

	static TileDefinition* GetTileDefinition( std::string tileName );

public:
	static std::map< std::string, TileDefinition* >	s_definitions;

private:
	bool m_isValid = false;
	std::string m_name;
	TileMaterialDefinition* m_matDef = nullptr;

	// Tile attributes
	bool m_isSolid = false;
};



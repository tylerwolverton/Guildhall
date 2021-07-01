#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class TileDefinition
{
	friend class Tile;

public:
	explicit TileDefinition( const XmlElement& tileDefElem);

	std::string GetName()										{ return m_name; }
	const AABB2 GetUVCoords()									{ return m_uvCoords; }
	const Rgba8 GetSpriteTint()									{ return m_spriteTint; }

	static TileDefinition* GetTileDefinition( std::string tileName );
	static TileDefinition* GetTileDefinitionFromImageTexelColor( const Rgba8& imageTexelColor );

public:
	static std::map< std::string, TileDefinition* >	s_definitions;

private:
	std::string m_name;
	AABB2		m_uvCoords = AABB2::ONE_BY_ONE;
	Rgba8		m_tileImageColor = Rgba8::BLACK;
	Rgba8		m_spriteTint = Rgba8::MAGENTA;
};



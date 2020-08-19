#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;


//-----------------------------------------------------------------------------------------------
class TileMaterialDefinition
{
	friend class TileDefinition;

public:
	explicit TileMaterialDefinition( const XmlElement& tileMatDefElem);

	std::string GetName() const									{ return m_name; }
	const AABB2 GetUVCoords() const								{ return m_uvCoords; }
	const Rgba8 GetSpriteTint()	const							{ return m_spriteTint; }
	SpriteSheet* GetSpriteSheet() const							{ return m_sheet; }
	IntVec2 GetSpriteCoords() const								{ return m_spriteCoords; }

	static TileMaterialDefinition* GetTileMaterialDefinition( std::string tileMaterialName );

public:
	static std::map< std::string, TileMaterialDefinition* >	s_definitions;

private:
	bool m_isValid = false;
	std::string		m_name;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;
	Rgba8			m_spriteTint = Rgba8::MAGENTA;
	SpriteSheet*	m_sheet = nullptr;
	IntVec2			m_spriteCoords = IntVec2( -1, -1 );
};

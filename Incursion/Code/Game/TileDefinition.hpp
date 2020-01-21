#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class TileDefinition
{
public:
	TileDefinition( const AABB2& uvCoords, const Rgba8& tintColor, bool isSolid, bool stopsBullets = false );
	~TileDefinition();

	static void PopulateDefinitions();

	AABB2 GetUVCoords()				{ return m_uvCoords; }
	Rgba8 GetTintColor()			{ return m_tintColor; }
	bool IsSolid()					{ return m_isSolid; }
	bool StopsBullets()				{ return m_stopsBullets; }

public:
	static std::vector<TileDefinition> s_definitions;

private:
	AABB2 m_uvCoords;
	Rgba8 m_tintColor = Rgba8( 255, 255, 255, 0 );
	bool m_isSolid = false;
	bool m_stopsBullets = false;
};

#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapRegionTypeDefinition.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
struct AABB2;


//-----------------------------------------------------------------------------------------------
class Tile
{
public:
	Tile( IntVec2 tileCoords, TileDefinition* tileDef );
	Tile( IntVec2 tileCoords, MapRegionTypeDefinition* regionTypeDef );
	Tile( IntVec2 tileCoords, bool isSolid = false );
	Tile( int x, int y, TileDefinition* tileDef );
	~Tile();

	std::string GetName() const										{ return m_tileDef->GetName(); }
	AABB2		GetBounds() const;
	bool		AllowsWalking() const;
	bool		AllowsSwimming() const;
	bool		AllowsFlying() const;

	void		SetTileDef( TileDefinition* tileDef )				{ m_tileDef = tileDef; }

	bool		IsSolid() const										{ return m_regionTypeDef->m_isSolid; }

public:
	IntVec2			m_tileCoords;
	TileDefinition* m_tileDef = nullptr;
	MapRegionTypeDefinition* m_regionTypeDef = nullptr;
};

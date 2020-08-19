#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Game/TileDefinition.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
struct AABB2;


//-----------------------------------------------------------------------------------------------
class Tile
{
public:
	Tile( IntVec2 tileCoords, TileDefinition* tileDef );
	Tile( int x, int y, TileDefinition* tileDef );
	~Tile();

	std::string GetName() const										{ return m_tileDef->GetName(); }
	AABB2		GetBounds() const;

	bool IsSolid() const											{ return m_tileDef->m_isSolid; }

	void		SetTileDef( TileDefinition* tileDef )				{ m_tileDef = tileDef; }

	TileMaterialDefinition* GetTileMaterialDef() const				{ return m_tileDef->m_matDef; }

public:
	IntVec2			m_tileCoords;
	TileDefinition* m_tileDef = nullptr;
};

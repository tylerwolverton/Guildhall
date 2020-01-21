#pragma once
#include "Engine/Math/IntVec2.hpp"

class Tile
{
public:
	Tile( IntVec2  tileCoords );
	~Tile();
	void Render() const;
	
public:
	IntVec2 m_tileCoords;

};
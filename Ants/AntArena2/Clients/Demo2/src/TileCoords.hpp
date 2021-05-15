#pragma once


//------------------------------------------------------------------------------------------------
struct TileCoords;
extern TileCoords TILE_COORDS_INVALID;


//------------------------------------------------------------------------------------------------
struct TileCoords
{
public:
	short x = -999;
	short y = -999;

public:
	TileCoords() = default;
	TileCoords( short initialX, short initialY )
		: x( initialX )
		, y( initialY )
	{}

	const TileCoords operator+( const TileCoords& rhs ) const;

	void Set( short newX, short newY )
	{
		x = newX;
		y = newY;
	}
};



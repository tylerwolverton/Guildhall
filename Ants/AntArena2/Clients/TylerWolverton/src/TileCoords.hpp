#pragma once


//------------------------------------------------------------------------------------------------
struct TileCoords;
extern TileCoords TILE_COORDS_INVALID;


//------------------------------------------------------------------------------------------------
struct TileCoords
{
public:
	int x = -999;
	int y = -999;

public:
	TileCoords() = default;
	TileCoords( int initialX, int initialY )
		: x( initialX )
		, y( initialY )
	{}

	const TileCoords operator+( const TileCoords& rhs ) const;
	const TileCoords operator-( const TileCoords& rhs ) const;
	bool operator==( const TileCoords& rhs ) const;
	bool operator!=( const TileCoords& rhs ) const;

	int GetTaxicabLength() const;

	void Set( int newX, int newY )
	{
		x = newX;
		y = newY;
	}
};



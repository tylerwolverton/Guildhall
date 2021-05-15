#include "TileCoords.hpp"

#include <math.h>


TileCoords TILE_COORDS_INVALID( -999, -999 );


//------------------------------------------------------------------------------------------------
const TileCoords TileCoords::operator+( const TileCoords& rhs ) const
{
	return TileCoords( x + rhs.x, y + rhs.y );
}


//-----------------------------------------------------------------------------------------------
const TileCoords TileCoords::operator-( const TileCoords& rhs ) const
{
	return TileCoords( x - rhs.x, y - rhs.y );
}


//-----------------------------------------------------------------------------------------------
int TileCoords::GetTaxicabLength() const
{
	return abs( (int)x ) + abs( (int)y );
}


//-----------------------------------------------------------------------------------------------
bool TileCoords::operator!=( const TileCoords& rhs ) const
{
	return !( *this == rhs );
}


//-----------------------------------------------------------------------------------------------
bool TileCoords::operator==( const TileCoords& rhs ) const
{
	return x == rhs.x && y == rhs.y;
}



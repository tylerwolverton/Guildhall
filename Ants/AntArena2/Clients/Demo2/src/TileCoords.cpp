#include "TileCoords.hpp"


TileCoords TILE_COORDS_INVALID( -999, -999 );


//------------------------------------------------------------------------------------------------
const TileCoords TileCoords::operator+( const TileCoords& rhs ) const
{
	return TileCoords( x + rhs.x, y + rhs.y );
}



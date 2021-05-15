#include "Common.hpp"
#include <varargs.h>


//------------------------------------------------------------------------------------------------
void Errorf( const char* format, ... )
{
	if( !g_debug )
		return;

	const int MESSAGE_MAX_LENGTH = 2048;
	char messageLiteral[ MESSAGE_MAX_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList ); // Note: only compiles in x64
	vsnprintf_s( messageLiteral, MESSAGE_MAX_LENGTH, _TRUNCATE, format, variableArgumentList );
	va_end( variableArgumentList );
	messageLiteral[ MESSAGE_MAX_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	g_debug->LogText( "%s", messageLiteral );
}


//------------------------------------------------------------------------------------------------
// Does the report herald the arrival of a new Ant we need to add internally?
//
bool IsBirthReport( eAgentOrderResult orderResult )
{
	switch( orderResult )
	{
		case AGENT_WAS_CREATED:
			return true;

		default:
			return false;
	}

}


//------------------------------------------------------------------------------------------------
// Is the report indicating that this Ant died (for whatever reason)?
//
bool IsDeathReport( eAgentOrderResult orderResult )
{
	switch( orderResult )
	{
		case AGENT_KILLED_BY_ENEMY:
		case AGENT_KILLED_BY_WATER:
		case AGENT_KILLED_BY_SUFFOCATION:
		case AGENT_KILLED_BY_STARVATION:
		case AGENT_KILLED_BY_PENALTY:
		case AGENT_ORDER_SUCCESS_SUICIDE:
			return true;

		default:
			return false;
	}
}


//------------------------------------------------------------------------------------------------
// Is the given orderResult something caused by our own stupidity / incorrectness?
//
bool IsStupidityReport( eAgentOrderResult orderResult )
{
	switch( orderResult )
	{
		case AGENT_KILLED_BY_PENALTY:
		case AGENT_ORDER_ERROR_BAD_ANT_ID:
		case AGENT_ORDER_ERROR_EXHAUSTED:
		case AGENT_ORDER_ERROR_CANT_CARRY_FOOD:
		case AGENT_ORDER_ERROR_CANT_CARRY_TILE:
		case AGENT_ORDER_ERROR_CANT_BIRTH:
		case AGENT_ORDER_ERROR_CANT_DIG_INVALID_TILE:
		case AGENT_ORDER_ERROR_CANT_DIG_WHILE_CARRYING:
		case AGENT_ORDER_ERROR_MOVE_BLOCKED_BY_TILE:
		case AGENT_ORDER_ERROR_OUT_OF_BOUNDS:
		case AGENT_ORDER_ERROR_NO_FOOD_PRESENT:
		case AGENT_ORDER_ERROR_ALREADY_CARRYING_FOOD:
		case AGENT_ORDER_ERROR_NOT_CARRYING:
		case AGENT_ORDER_ERROR_INSUFFICIENT_FOOD:
		case AGENT_ORDER_ERROR_MAXIMUM_POPULATION_REACHED:
			return true;

		default:
			return false;
	}
}


//-----------------------------------------------------------------------------------------------
TileCoords GetTileCoordsFromTileIdx( int tileIdx, int mapWidth )
{
	int x = tileIdx % mapWidth;
	int y = tileIdx / mapWidth;

	return TileCoords( x, y );
}


//-----------------------------------------------------------------------------------------------
int GetTileIdxFromTileCoords( const TileCoords& tileCoords, int mapWidth )
{
	if ( tileCoords.x < 0 || tileCoords.x > mapWidth - 1
		 || tileCoords.y < 0 || tileCoords.y > mapWidth - 1 )
	{
		return -1;
	}

	return tileCoords.y * mapWidth + tileCoords.x;
}


//-----------------------------------------------------------------------------------------------
TileCoords GetAdjacentTile( const TileCoords& tileCoords, eCardinalDirections direction, int mapWidth )
{
	if ( tileCoords.x < 0 || tileCoords.x > mapWidth - 1
		 || tileCoords.y < 0 || tileCoords.y > mapWidth - 1 )
	{
		return TILE_COORDS_INVALID;
	}

	switch ( direction )
	{
		case EAST:	return tileCoords + TileCoords( 1, 0 );
		case WEST:	return tileCoords - TileCoords( 1, 0 );
		case NORTH:	return tileCoords + TileCoords( 0, 1 );
		case SOUTH:	return tileCoords - TileCoords( 0, 1 );
	}

	return TILE_COORDS_INVALID;
}


//-----------------------------------------------------------------------------------------------
int GetAdjacentTileIdx( const TileCoords& tileCoords, eCardinalDirections direction, int mapWidth )
{
	switch ( direction )
	{
		case EAST:	return GetTileIdxFromTileCoords( tileCoords + TileCoords( 1, 0 ), mapWidth );
		case WEST:	return GetTileIdxFromTileCoords( tileCoords - TileCoords( 1, 0 ), mapWidth );
		case NORTH:	return GetTileIdxFromTileCoords( tileCoords + TileCoords( 0, 1 ), mapWidth );
		case SOUTH:	return GetTileIdxFromTileCoords( tileCoords - TileCoords( 0, 1 ), mapWidth );
	}

	return -1;
}

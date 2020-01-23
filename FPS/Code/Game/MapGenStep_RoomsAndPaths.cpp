#include "Game/MapGenStep_RoomsAndPaths.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/TileDefinition.hpp"

//-----------------------------------------------------------------------------------------------
MapGenStep_RoomsAndPaths::MapGenStep_RoomsAndPaths( const XmlElement& mapGenStepXmlElem )
	: MapGenStep( mapGenStepXmlElem )
	, m_numRooms( 3, 7 )
	, m_roomWidth( 4, 8 )
	, m_roomHeight( 4, 8 )
	, m_numOverlaps( 0 )
	, m_chanceForLoop( 0.f )
	, m_pathStraightness( 0.f, 1.f )
	, m_numExtraPaths( 0 )
{
	// Room variables
	const XmlElement* roomElem = mapGenStepXmlElem.FirstChildElement( "Rooms" );
	GUARANTEE_OR_DIE( roomElem != nullptr, "No Rooms node found. MapGenStep_RoomsAndPaths must have a Rooms sub element" );

	std::string floorTileName;
	floorTileName = ParseXmlAttribute( *roomElem, "floorTile", floorTileName );
	m_floorType = TileDefinition::GetTileDefinition( floorTileName );
	GUARANTEE_OR_DIE( m_floorType != nullptr, "No floorTile defined. MapGenStep_RoomsAndPaths must have a floorTile" );

	std::string wallTileName;
	wallTileName = ParseXmlAttribute( *roomElem, "wallTile", wallTileName );
	m_wallType = TileDefinition::GetTileDefinition( wallTileName );
	GUARANTEE_OR_DIE( m_wallType != nullptr, "No wallTile defined. MapGenStep_RoomsAndPaths must have a wallTile" );

	m_numRooms = ParseXmlAttribute( *roomElem, "count", m_numRooms );
	m_roomWidth = ParseXmlAttribute( *roomElem, "width", m_roomWidth );
	m_roomHeight = ParseXmlAttribute( *roomElem, "height", m_roomHeight );
	m_numOverlaps = ParseXmlAttribute( *roomElem, "numOverlaps", m_numOverlaps );

	// Path variables
	const XmlElement* pathElem = mapGenStepXmlElem.FirstChildElement( "Paths" );
	GUARANTEE_OR_DIE( pathElem != nullptr, "No Paths node found. MapGenStep_RoomsAndPaths must have a Paths sub element" );
	
	std::string pathTileName;
	pathTileName = ParseXmlAttribute( *pathElem, "pathTile", pathTileName );
	m_pathType = TileDefinition::GetTileDefinition( pathTileName );
	GUARANTEE_OR_DIE( m_pathType != nullptr, "No pathTile defined. MapGenStep_RoomsAndPaths must have a pathTile" );

	m_chanceForLoop = ParseXmlAttribute( *pathElem, "chanceForLoop", m_chanceForLoop );
	m_pathStraightness = ParseXmlAttribute( *pathElem, "straightness", m_pathStraightness );
	m_numExtraPaths = ParseXmlAttribute( *pathElem, "numExtraPaths", m_numExtraPaths );
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_RoomsAndPaths::RunStepOnce( Map& map )
{
	std::vector<AABB2> existingRoomBounds;
	BuildRooms( map, existingRoomBounds );
	BuildPaths( map, existingRoomBounds );
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_RoomsAndPaths::BuildRooms( Map& map, std::vector<AABB2>& existingRoomBounds )
{
	int numRooms = m_numRooms.GetRandomInRange( g_game->m_rng );
	int numOverlaps = m_numOverlaps.GetRandomInRange( g_game->m_rng );

	AABB2 mapBounds( 0.f, 0.f, (float)map.m_width, (float)map.m_height );

	for ( int roomNum = 0; roomNum < numRooms; ++roomNum )
	{
		int roomWidth = m_roomWidth.GetRandomInRange( g_game->m_rng );
		int roomHeight = m_roomHeight.GetRandomInRange( g_game->m_rng );

		bool canOverlap = numOverlaps > 0;

		AABB2 roomBounds = GetBoundsForNewRoom( roomWidth, roomHeight, mapBounds, canOverlap, existingRoomBounds );
		existingRoomBounds.push_back( roomBounds );
		IntVec2 roomMins = IntVec2( (int)roomBounds.mins.x, (int)roomBounds.mins.y );

		SetRoomTiles( map, roomWidth, roomHeight, roomMins );

	}
}


//-----------------------------------------------------------------------------------------------
const AABB2 MapGenStep_RoomsAndPaths::GetBoundsForNewRoom( int roomWidth, int roomHeight, const AABB2& mapBounds, bool canOverlap, const std::vector<AABB2>& existingRoomBounds )
{
	AABB2 roomBounds;
	bool isRoomPosValid = false;
	int attemptCount = 0;
	while ( !isRoomPosValid && attemptCount < 1000 )
	{
		int rndX = g_game->m_rng->RollRandomIntLessThan( (int)mapBounds.maxs.x );
		int rndY = g_game->m_rng->RollRandomIntLessThan( (int)mapBounds.maxs.y );
		Vec2 roomMins( (float)rndX, (float)rndY );

		Vec2 roomMaxs( roomMins );
		roomMaxs.x += roomWidth;
		roomMaxs.y += roomHeight;

		roomBounds = AABB2( roomMins, roomMaxs );
		roomBounds.FitWithinBounds( mapBounds );

		if ( canOverlap )
		{
			isRoomPosValid = true;
		}
		else
		{
			bool newRoomIsOverlapping = false;
			for ( int existingRoomIdx = 0; existingRoomIdx < (int)existingRoomBounds.size(); ++existingRoomIdx )
			{
				if ( roomBounds.OverlapsWith( existingRoomBounds[existingRoomIdx] ) )
				{
					newRoomIsOverlapping = true;
					break;
				}
			}

			if ( newRoomIsOverlapping )
			{
				++attemptCount;
			}
			else
			{
				isRoomPosValid = true;
			}
		}
	}

	return roomBounds;
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_RoomsAndPaths::SetRoomTiles( Map& map, int roomWidth, int roomHeight, const IntVec2& roomMins )
{
	for ( int yCoord = 0; yCoord < roomHeight; ++yCoord )
	{
		for ( int xCoord = 0; xCoord < roomWidth; ++xCoord )
		{
			Tile* tile = map.GetTileFromTileCoords( roomMins + IntVec2( xCoord, yCoord ) );
			if ( tile != nullptr )
			{
				if ( tile->m_tileDef != m_floorType
					 && ( xCoord == 0
						  || xCoord == roomWidth - 1
						  || yCoord == 0
						  || yCoord == roomHeight - 1 ) )
				{
					tile->SetTileDef( m_wallType );
				}
				else
				{
					tile->SetTileDef( m_floorType );
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_RoomsAndPaths::BuildPaths( Map& map, std::vector<AABB2> existingRoomBounds )
{
	// Need multiple rooms for paths
	int numRooms = (int)existingRoomBounds.size();
	if ( numRooms < 2 )
	{
		return;
	}

	// TODO: Sort rooms in counterclockwise order?

	for ( int roomIndex = 0; roomIndex < numRooms - 1; ++roomIndex )
	{
		int nextRoomIndex = roomIndex + 1;
		ConnectRoomsWithPath( map, existingRoomBounds[ roomIndex ], existingRoomBounds[ nextRoomIndex ] );
	}
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_RoomsAndPaths::ConnectRoomsWithPath( Map& map, const AABB2& room1, const AABB2& room2 )
{
	// No path needed if the rooms overlap
	if ( room1.OverlapsWith( room2 ) )
	{
		return;
	}

	Vec2 pathStart;
	Vec2 pathEnd;
	Vec2 pathStep( 1.f, 1.f );

	PopulatePathParams( room1, room2, pathStart, pathEnd, pathStep );

	float straightness = m_pathStraightness.GetRandomInRange( g_game->m_rng );
	Vec2 curPos( pathStart );
	IntVec2 remainingPathLength( (int)fabsf( pathEnd.x - pathStart.x ), (int)fabsf( pathEnd.y - pathStart.y ) );

	int totalLength = remainingPathLength.GetTaxicabLength();
	for ( int tileNum = 0; tileNum < totalLength; ++tileNum )
	{
		if ( remainingPathLength.x == 0 )
		{
			AddVerticalPathTile( map, curPos, pathStep.y, remainingPathLength );
			continue;
		}

		if ( remainingPathLength.y == 0 )
		{
			AddHorizontalPathTile( map, curPos, pathStep.x, remainingPathLength );
			continue;
		}

		if ( g_game->m_rng->RollPercentChance( straightness ) )
		{
			AddHorizontalPathTile( map, curPos, pathStep.x, remainingPathLength );
		}
		else
		{
			AddVerticalPathTile( map, curPos, pathStep.y, remainingPathLength );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_RoomsAndPaths::PopulatePathParams( const AABB2& room1, const AABB2& room2, Vec2& out_pathStart, Vec2& out_pathEnd, Vec2& out_pathStep )
{
	Vec2 room1Doors[4];
	Vec2 room2Doors[4];

	room1.GetPositionOnEachEdge( .5f, room1Doors );
	room2.GetPositionOnEachEdge( .5f, room2Doors );

	Vec2 potentialStart;
	Vec2 potentialEnd;
	float minLength = 999999.f;

	for ( int room1DoorIndex = 0; room1DoorIndex < 4; ++room1DoorIndex )
	{
		for ( int room2DoorIndex = 0; room2DoorIndex < 4; ++room2DoorIndex )
		{
			float distance = GetDistanceSquared2D( room1Doors[ room1DoorIndex ], room2Doors[ room2DoorIndex ] );
			if ( distance < minLength )
			{
				potentialStart = room1Doors[ room1DoorIndex ];
				potentialEnd = room2Doors[ room2DoorIndex ];
				minLength = distance;
			}
		}
	}

	out_pathStart = potentialStart;
	out_pathEnd = potentialEnd;

	if ( out_pathStart.x > out_pathEnd.x )
	{
		out_pathStep.x *= -1.f;
	}
	if ( out_pathStart.y > out_pathEnd.y )
	{
		out_pathStep.y *= -1.f;
	}
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_RoomsAndPaths::AddHorizontalPathTile( Map& map, Vec2& curPos, float nextTileStep, IntVec2& remainingPathLength )
{
	curPos.x += nextTileStep;
	--remainingPathLength.x;
	Tile* nextPathTile = map.GetTileFromWorldCoords( curPos );
	if ( nextPathTile == nullptr )
	{
		return;
	}

	nextPathTile->SetTileDef( m_pathType );
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_RoomsAndPaths::AddVerticalPathTile( Map& map, Vec2& curPos, float nextTileStep, IntVec2& remainingPathLength )
{
	curPos.y += nextTileStep;
	--remainingPathLength.y;
	Tile* nextPathTile = map.GetTileFromWorldCoords( curPos );
	if ( nextPathTile == nullptr )
	{
		return;
	}

	nextPathTile->SetTileDef( m_pathType );
}

#pragma once
#include "Game/MapGenStep.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"


//-----------------------------------------------------------------------------------------------
struct AABB2;


//-----------------------------------------------------------------------------------------------
class MapGenStep_RoomsAndPaths : public MapGenStep
{
public:
	MapGenStep_RoomsAndPaths( const XmlElement& mapGenStepXmlElem );
	virtual void RunStepOnce( Map& map ) override;

private:
	void		BuildRooms( Map& map, std::vector<AABB2>& existingRoomBounds );
	const AABB2 GetBoundsForNewRoom( int roomWidth, int roomHeight, const AABB2& mapBounds, bool canOverlap, const std::vector<AABB2>& existingRoomBounds );
	void		SetRoomTiles( Map& map, int roomWidth, int roomHeight, const IntVec2& roomMins );

	void		BuildPaths( Map& map, std::vector<AABB2> existingRoomBounds );
	void		ConnectRoomsWithPath( Map& map, const AABB2& room1, const AABB2& room2 );
	void		PopulatePathParams( const AABB2& room1, const AABB2& room2, Vec2& out_pathStart, Vec2& out_pathEnd, Vec2& out_pathStep );
	void		AddHorizontalPathTile( Map& map, Vec2& curPos, float nextTileStep, IntVec2& remainingPathLength );
	void		AddVerticalPathTile( Map& map, Vec2& curPos, float nextTileStep, IntVec2& remainingPathLength );


private:
	IntRange m_numRooms;
	IntRange m_roomWidth;
	IntRange m_roomHeight;
	IntRange m_numOverlaps;
	TileDefinition* m_floorType = nullptr;
	TileDefinition* m_wallType = nullptr;

	TileDefinition* m_pathType = nullptr;
	FloatRange m_chanceForLoop;
	FloatRange m_pathStraightness;
	IntRange m_numExtraPaths;

};

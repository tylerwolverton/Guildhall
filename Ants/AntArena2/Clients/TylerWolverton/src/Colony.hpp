#pragma once
#include "Common.hpp"
#include "Ant.hpp"

#include <vector>


//------------------------------------------------------------------------------------------------
struct Tile
{
public:
	Ant*				assignedAnt = nullptr;
	std::vector<eAgentType>	antsInTile;
	
	int					lastTurnSeen = -1;

	eTileType			type = TILE_TYPE_UNSEEN;
	bool				hasFood = false;
};


//------------------------------------------------------------------------------------------------
class Colony;
struct IntVec2;


//------------------------------------------------------------------------------------------------
extern Colony* g_colony;


//------------------------------------------------------------------------------------------------
class Colony
{
	friend struct Ant;

public:
	Colony( StartupInfo const& startupInfo );

	void Update();
	void ProcessNewArenaUpdate();
	void AssignBasicOrdersToAnts();
	void UpdateOrdersFromAnts();

private:
	Ant* GetMyAntForID( AgentID antID );
	void ProcessGeneralTurnInfo();
	void ProcessVisibleTilesAndFood();
	void ProcessVisibleAgents();
	void ProcessDeathReports();
	void ProcessBirthReports();
	void ProcessOtherReports();
	void ProcessReportForAnt( AgentReport const& report, Ant& ant );
	void ProcessCompletedPaths();

	void OnAntDied( Ant* ant );
	void OnAntBorn( AgentReport const& report );

	bool SpawnNewAnt( Ant& queen );

	void MoveTowardsGoal( Ant& ant );
	void MoveTowardsGoalQueen( Ant& ant );
	void MoveTowardsGoalSoldier( Ant& ant );
	
	void PathToGoal( Ant& ant );
	
	void PickNewGoal( Ant& ant );
	void PickNewGoalCollectFood( Ant& ant );
	void PickNewGoalDefendQueen( Ant& ant );
	void PickNewGoalAttackQueen( Ant& ant );
	void PickNewGoalScout( Ant& ant );

	TileCoords FindNearestUnclaimedFood( const Ant& ant, const std::vector<eTileType> validTileTypes, int minDistThreshhold );
	TileCoords FindNextQueenGoal( const Ant& ant );

	int			GetRandomValidTileIdx( const Ant& ant ) const;
	TileCoords	GetNearestQueenLocation( const TileCoords& startTile );
	TileCoords	GetNearestEnemyLocation( const TileCoords& startTile, int maxDist );
	TileCoords	GetNearestStationaryQueenLocation( const TileCoords& startTile );
	TileCoords	GetNearestEnemyQueenLocation( const TileCoords& startTile );
	Ant*		GetRandomQueen();
	Ant*		GetRandomStationaryQueen();

	void		AbandonGoal( Ant& ant );
	void		ResetAnt( Ant& ant );

	TileCoords  GetTileCoordsFromTileIdx( int tileIdx ) const;
	int			GetTileIdxFromTileCoords( const TileCoords& tileCoords ) const;
	TileCoords	GetAdjacentTile( const TileCoords& tileCoords, eCardinalDirections direction ) const;
	int			GetAdjacentTileIdx( const TileCoords& tileCoords, eCardinalDirections direction ) const;
	int			GetAdjacentTileIdx( int tileIdx, eCardinalDirections direction ) const;
	bool		IsTileSafeForAnt( const TileCoords& tileCoords, const Ant& ant ) const;

	void		RequestPathToGoalForAnt( Ant& ant );

private:
	// Note ALL Colony data (unless otherwise specified) is used (read and written) only by the primary DLL thread (#0).
	PlayerID					m_playerID = INVALID_PLAYER_ID;
	TeamID						m_teamID = INVALID_TEAM_ID;
	Color8						m_color = Color8( 255, 0, 255, 255 );
	unsigned char				m_teamSize = 0;
	int							m_numQueens = 0;
	int							m_numWorkers = 0;
	int							m_numSoldiers = 0;
	int							m_numScouts = 0;

	int							m_turnNumInfoLastProcessed = -1;
	int							m_turnNumOrdersLastPosted = -1;
	int							m_nutrients = 0;
	std::map< AgentID, Ant*>	m_myAntsByID;
	std::map< AgentID, TileCoords>	m_enemyQueensByID;
	std::vector<Ant*>			m_myQueens;
//	???							m_latestMap;


	eTileType					m_observedTileTypes[MAX_ARENA_TILES];
	Tile						m_observedTiles[MAX_ARENA_TILES];
	ObservedAgent				m_observedAgents[MAX_AGENTS_TOTAL];
	StartupInfo					m_startupInfo;
	ArenaTurnStateForPlayer		m_turnInfo;
};


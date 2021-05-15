#pragma once
#include "Common.hpp"


//------------------------------------------------------------------------------------------------
class Colony;


//------------------------------------------------------------------------------------------------
extern Colony* g_colony;


//------------------------------------------------------------------------------------------------
struct Ant // #ToDo: move this to Ant.cpp/hpp as it expands
{
	AgentID				m_id = INVALID_AGENT_ID;
	eAgentType			m_type = INVALID_AGENT_TYPE;
	TileCoords			m_pos = TILE_COORDS_INVALID;
	eOrderCode			m_nextOrder = ORDER_HOLD;
	eAgentOrderResult	m_lastResult = AGENT_WAS_CREATED;
	eAgentState			m_lastState = STATE_NORMAL;
	TileCoords			m_goalPos = TILE_COORDS_INVALID;
//	Path*				m_pathToGoal = nullptr;
//	???
};


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
	void ProcessDeathReports();
	void ProcessBirthReports();
	void ProcessOtherReports();
	void ProcessReportForAnt( AgentReport const& report, Ant& ant );

//	void OnAntDied( Ant* ant );
	void OnAntBorn( AgentReport const& report );

private:
	// Note ALL Colony data (unless otherwise specified) is used (read and written) only by the primary DLL thread (#0).
	PlayerID					m_playerID = INVALID_PLAYER_ID;
	TeamID						m_teamID = INVALID_TEAM_ID;
	Color8						m_color = Color8( 255, 0, 255, 255 );
	unsigned char				m_teamSize = 0;

	int							m_turnNumInfoLastProcessed = -1;
	int							m_turnNumOrdersLastPosted = -1;
	int							m_nutrients = 0;
	std::map< AgentID, Ant*>	m_myAntsByID;
//	???							m_latestMap;

	StartupInfo					m_startupInfo;
	ArenaTurnStateForPlayer		m_turnInfo;
};




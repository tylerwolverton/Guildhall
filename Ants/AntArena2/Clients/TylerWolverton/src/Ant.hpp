#pragma once
#include "Common.hpp"

#include <map>


//------------------------------------------------------------------------------------------------
enum eJob
{
	NO_JOB = -1,
	COLLECT_FOOD,
	STATIONARY_QUEEN,
	DEFEND_QUEEN,
	ATTACK_QUEEN,
	SCOUT,
};


//------------------------------------------------------------------------------------------------
struct Ant
{
	AgentID					m_id = INVALID_AGENT_ID;
	eAgentType				m_type = INVALID_AGENT_TYPE;
	TileCoords				m_pos = TILE_COORDS_INVALID;
	eOrderCode				m_nextOrder = ORDER_HOLD;
	eAgentOrderResult		m_lastResult = AGENT_WAS_CREATED;
	eAgentState				m_lastState = STATE_NORMAL;
	TileCoords				m_goalPos = TILE_COORDS_INVALID;
	bool					m_hasFood = false;
	int						m_exhaustion = 0;
	int						m_previouslyVisitedTileCost[4] = { 0 };
	std::vector<TileCoords>	m_abandonedGoals;
	std::map<int, int>	m_recentTileCosts;
	Path					m_pathToGoal;
	TileCoords				m_nextPathStep = TILE_COORDS_INVALID;
	TileCoords				m_lastPos = TILE_COORDS_INVALID;
	int						m_turnsTryingToReachGoal = 0;
	int						m_turnsInTheSamePlace = 0;
	int						m_maxTurnsTryingToReachGoal = 30;
	eJob					m_job = NO_JOB;
	bool					m_receivedCombatDamage = false;
	bool					m_waitingOnPath = false;
	//	???
};


#include "Colony.hpp"
#include "ThreadSafeStructures.hpp"
#include "RenderUtils.hpp"
#include "Math/IntVec2.hpp"
#include "Math/Vec2.hpp"

#include <deque>


//------------------------------------------------------------------------------------------------
Colony* g_colony = nullptr;


//------------------------------------------------------------------------------------------------
Colony::Colony( const StartupInfo& startupInfo )
	: m_startupInfo( startupInfo )
{
	m_playerID = m_startupInfo.yourPlayerInfo.playerID;
	m_teamSize = m_startupInfo.yourPlayerInfo.teamSize;
	m_teamID = m_startupInfo.yourPlayerInfo.teamID;
	m_color = m_startupInfo.yourPlayerInfo.color;

	// Initialize all observed tiles to unseen
	for ( int tileIdx = 0; tileIdx < MAX_ARENA_TILES; ++tileIdx )
	{
		m_observedTileTypes[tileIdx] = TILE_TYPE_UNSEEN;		
	}
}


//------------------------------------------------------------------------------------------------
void Colony::Update()
{
	if( g_threadSafe_turnStatus.Get() == TURN_STATUS_PROCESSING_UPDATE )
	{
		g_colony->ProcessNewArenaUpdate();
		g_threadSafe_turnStatus.Set( TURN_STATUS_WORKING_ON_ORDERS );
		g_colony->AssignBasicOrdersToAnts();
		g_threadSafe_turnStatus.Set( TURN_STATUS_ORDERS_READY );
		//g_colony->AssignAdvancedOrdersToAnts();
	}
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessNewArenaUpdate()
{
	g_threadSafe_turnInfo.CopyTo( m_turnInfo );

	ProcessGeneralTurnInfo();
	ProcessVisibleTilesAndFood();
	ProcessVisibleAgents();
	ProcessDeathReports();
	ProcessBirthReports();
	ProcessOtherReports();

	ProcessCompletedPaths();
}


//------------------------------------------------------------------------------------------------
void Colony::AssignBasicOrdersToAnts()
{
	// #ToDo: loop through every Ant in the colony and determine its best "obvious" (no/low-cpu) order.
	//  If you have a path, follow it.  If you have a goal but no path, move toward it, etc.

	int upkeepCost = 0;
	std::vector<VertexPC> verts;
	for ( std::map< AgentID, Ant*>::iterator antIter = m_myAntsByID.begin(); antIter != m_myAntsByID.end(); ++antIter )
	{
		Ant* ant = antIter->second;
		if ( !ant || ant->m_lastState == STATE_DEAD )
		{
			continue;
		}

		upkeepCost += m_startupInfo.matchInfo.agentTypeInfos[ant->m_type].upkeepPerTurn;

		std::string antTypeStr;
		switch ( ant->m_type )
		{
			case AGENT_TYPE_QUEEN: antTypeStr = "Queen"; break;
			case AGENT_TYPE_SCOUT: antTypeStr = "Scout"; break;
			case AGENT_TYPE_SOLDIER: antTypeStr = "Soldier"; break;
			case AGENT_TYPE_WORKER: antTypeStr = "Worker"; break;
		}

		if ( ant->m_type == AGENT_TYPE_QUEEN )
		{
			if ( SpawnNewAnt( *ant ) )
			{
				continue;
			}
		}

		//g_debug->QueueDrawWorldText( ant->m_pos.x, ant->m_pos.y, 0.5f, 0.5f, .75f, Color8( 0, 150, 150 ), antTypeStr.c_str() );

		// #ToDo: write some actual function(s) to decide what ants should do (e.g. follow a path, pick up food, etc.)
		if ( ant->m_goalPos != TILE_COORDS_INVALID )
		{
			MoveTowardsGoal( *ant );
			//PathToGoal( *ant );
		}
		else
		{
			PickNewGoal( *ant );
		}
		
		
		AppendVertsForLine( verts, ant->m_pos, ant->m_goalPos, .15f, Color8( 150, 150, 0 ) );

		if ( !ant->m_pathToGoal.empty() )
		{
			std::vector<VertexPC> pathVerts;

			for ( int tileIdx = 0; tileIdx < (int)ant->m_pathToGoal.size(); ++tileIdx )
			{
				AppendVertsForTileOutline( pathVerts, ant->m_pathToGoal[tileIdx], .05f, Color8( 255, 255, 255 ) );
			}

			g_debug->QueueDrawVertexArray( (int)pathVerts.size(), pathVerts.data() );
		}
	}

	UpdateOrdersFromAnts(); // copy orders of all ants to shared ant orders array

	float x, y;
	g_debug->GetMouseWorldPos( x, y );
	Vec2 mousePos = Vec2( x, y );
	Vec2 endPos = Vec2( x, y ) + Vec2( 1.f, 1.f );
	AppendVertsForLine( verts, mousePos, endPos, .1f, Color8( 0, 255, 127 ) );

	g_debug->QueueDrawVertexArray( (int)verts.size(), verts.data() );

	g_debug->FlushQueuedDraws();
}


//------------------------------------------------------------------------------------------------
void Colony::UpdateOrdersFromAnts()
{
	PlayerTurnOrders newOrders;
	newOrders.numberOfOrders = 0;

	// Ask each ant to submit its current order into the newOrders array
	for( std::map< AgentID, Ant*>::iterator antIter = m_myAntsByID.begin(); antIter != m_myAntsByID.end(); ++ antIter )
	{
		Ant* ant = antIter->second;
		if( !ant || ant->m_lastState == STATE_DEAD )
		{
			continue;
		}

		// Add an order entry in the array for this ant (id and order)
		AgentOrder& order = newOrders.orders[ newOrders.numberOfOrders++ ];
		order.agentID = ant->m_id;
		order.order = ant->m_nextOrder;
	}

	g_threadSafe_turnOrders.CopyFrom( newOrders );
}


//------------------------------------------------------------------------------------------------
Ant* Colony::GetMyAntForID( AgentID antID )
{
	std::map< AgentID, Ant*>::iterator found = m_myAntsByID.find( antID );
	if( found != m_myAntsByID.end() )
	{
		return found->second;
	}
	else
	{
		return nullptr;
	}
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessGeneralTurnInfo()
{
	m_turnNumInfoLastProcessed = m_turnInfo.turnNumber;
	m_nutrients = m_turnInfo.currentNutrients;
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessVisibleTilesAndFood()
{
	// #ToDo: loop through visible tiles and food arrays provided; skip any TILE_TYPE_UNSEEN
	//	in the update map snapshot; update our internal map (and note whether food is present/absent)
	//	for all visible / currently seen tiles in the snapshot.
	for ( int tileIdx = 0; tileIdx < MAX_ARENA_TILES; ++tileIdx )
	{
		if ( m_turnInfo.observedTiles[tileIdx] == TILE_TYPE_UNSEEN )
		{
			continue;
		}

		m_observedTileTypes[tileIdx] = m_turnInfo.observedTiles[tileIdx];

		m_observedTiles[tileIdx].type = m_turnInfo.observedTiles[tileIdx];
		m_observedTiles[tileIdx].hasFood = m_turnInfo.tilesThatHaveFood[tileIdx];
		m_observedTiles[tileIdx].lastTurnSeen = m_turnInfo.turnNumber;
		m_observedTiles[tileIdx].antsInTile.clear();

		if ( m_observedTiles[tileIdx].hasFood == false )
		{
			m_observedTiles[tileIdx].assignedAnt = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Colony::ProcessVisibleAgents()
{
	int numObserved = 0;
	for ( int agentIdx = 0; agentIdx < MAX_AGENTS_TOTAL; ++agentIdx )
	{
		// Already seen everyone
		if ( numObserved == m_turnInfo.numObservedAgents )
		{
			return;
		}

		++numObserved;

		TileCoords tileCoords = TileCoords( m_turnInfo.observedAgents[agentIdx].tileX, m_turnInfo.observedAgents[agentIdx].tileY );
		int tileIdx = GetTileIdxFromTileCoords( tileCoords );
		m_observedTiles[tileIdx].antsInTile.push_back( m_turnInfo.observedAgents[agentIdx].type );

		if ( m_turnInfo.observedAgents[agentIdx].type == AGENT_TYPE_QUEEN )
		{
			m_enemyQueensByID[m_turnInfo.observedAgents[agentIdx].agentID] = tileCoords;
		}
	}
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessDeathReports()
{
	// #ToDo

	for( int reportIndex = 0; reportIndex < m_turnInfo.numReports; ++ reportIndex )
	{
		AgentReport& report = m_turnInfo.agentReports[ reportIndex ];
		if( IsDeathReport( report.result ) )
		{
			Ant* ant = GetMyAntForID( report.agentID );
			if( ant )
			{
				OnAntDied( ant );
			}
		}
	}
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessBirthReports()
{
	for( int reportIndex = 0; reportIndex < m_turnInfo.numReports; ++ reportIndex )
	{
		AgentReport& report = m_turnInfo.agentReports[ reportIndex ];
		if( IsBirthReport( report.result ) )
		{
			OnAntBorn( report );
		}
	}
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessOtherReports()
{
	for( int reportIndex = 0; reportIndex < m_turnInfo.numReports; ++ reportIndex )
	{
		AgentReport& report = m_turnInfo.agentReports[ reportIndex ];
		if( !IsBirthReport( report.result ) && !IsDeathReport( report.result ) )
		{
			Ant* ant = GetMyAntForID( report.agentID );
			if( !ant )
			{
				Errorf( "We got a report for an ant (#%d) that doesn't exist?", report.agentID );
			}
			else
			{
				ProcessReportForAnt( report, *ant );
			}
		}
	}
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessReportForAnt( AgentReport const& report, Ant& ant )
{
	ant.m_pos.Set( report.tileX, report.tileY );
	ant.m_exhaustion = report.exhaustion;
	ant.m_receivedCombatDamage = report.receivedCombatDamage;

	// Update ant based on what happened
	switch ( ant.m_lastState )
	{
		case STATE_NORMAL:
		{
			switch ( report.result )
			{
				case AGENT_ORDER_SUCCESS_PICKUP:
				{
					ResetAnt( ant );
					ant.m_hasFood = true;

					m_observedTiles[GetTileIdxFromTileCoords( ant.m_pos )].hasFood = false;
					ant.m_goalPos = GetNearestStationaryQueenLocation( ant.m_pos );
					RequestPathToGoalForAnt( ant );
				}
				break;
				
				case AGENT_ORDER_ERROR_NO_FOOD_PRESENT:
				{
					ResetAnt( ant );
				}
				break;
			}
		}
		break;

		case STATE_HOLDING_FOOD:
		{
			switch ( report.result )
			{
				case AGENT_ORDER_SUCCESS_DROP:
				{
					ResetAnt( ant );
					ant.m_hasFood = false;
					m_observedTiles[GetTileIdxFromTileCoords( ant.m_pos)].hasFood = true;
				}
				break;
			}
		}
		break;
	}

	ant.m_lastResult = report.result;
	ant.m_lastState = report.state;
}


//-----------------------------------------------------------------------------------------------
void Colony::ProcessCompletedPaths()
{
	PathCompleteData completeData;
	while ( g_completedPaths.Pop( completeData ) )
	{
		auto mapIter = m_myAntsByID.find( completeData.id );
		if ( mapIter != m_myAntsByID.end() )
		{
			Ant& ant = *mapIter->second;
  			/*if ( ant.m_type == AGENT_TYPE_QUEEN )
			{*/
				if ( completeData.pathToGoal.size() < 2 )
				{
					ant.m_abandonedGoals.push_back( ant.m_goalPos );
					ant.m_goalPos = TILE_COORDS_INVALID;
				}
				else
				{
					ant.m_abandonedGoals.clear();
				}
			//}

			ant.m_pathToGoal = completeData.pathToGoal;
			ant.m_waitingOnPath = false;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Colony::OnAntDied( Ant* ant )
{
	switch ( ant->m_type )
	{
		case AGENT_TYPE_QUEEN:
		{
			--m_numQueens;

			for ( int queenIdx = 0; queenIdx < (int)m_myQueens.size(); ++queenIdx )
			{
				if ( m_myQueens[queenIdx]->m_id == ant->m_id )
				{
					m_myQueens[queenIdx] = m_myQueens[m_myQueens.size() - 1];
					m_myQueens.pop_back();
					break;
				}
			}
		}
		break;

		case AGENT_TYPE_WORKER:
		{
			--m_numWorkers;
			m_observedTiles[GetTileIdxFromTileCoords( ant->m_goalPos )].assignedAnt = nullptr;
		}
		break;

		case AGENT_TYPE_SOLDIER:
		{
			--m_numSoldiers;
		}
		break;

		case AGENT_TYPE_SCOUT:
		{
			--m_numScouts;
		}
		break;
	}

	m_myAntsByID.erase( ant->m_id );
	--m_teamSize;
}


//------------------------------------------------------------------------------------------------
void Colony::OnAntBorn( const AgentReport& report )
{
	Ant* alreadyExistingAnt = GetMyAntForID( report.agentID );
	if( alreadyExistingAnt )
	{
		Errorf( "OnAntBorn called for agent #%i which already exists!\n", report.agentID );
		return;
	}

	Ant* newAnt = new Ant();
	newAnt->m_type = report.type;
	newAnt->m_id = report.agentID;
	newAnt->m_pos.Set( report.tileX, report.tileY );
	newAnt->m_goalPos = TILE_COORDS_INVALID;
	newAnt->m_lastResult = report.result;
	newAnt->m_lastState = report.state;

	m_myAntsByID[ report.agentID ] = newAnt;

	switch ( report.type )
	{
		case AGENT_TYPE_QUEEN:
		{
			++m_numQueens;
			
			if ( m_numQueens % 2 == 0 )
			{
				newAnt->m_job = STATIONARY_QUEEN;
			}
			else
			{
				newAnt->m_job = COLLECT_FOOD;
			}
			m_myQueens.push_back( newAnt );

		}
		break;

		case AGENT_TYPE_WORKER:
		{
			++m_numWorkers;
			newAnt->m_job = COLLECT_FOOD;
		}
		break;

		case AGENT_TYPE_SOLDIER:
		{
			++m_numSoldiers;
			/*if ( m_numSoldiers % 3 == 0 )
			{
				newAnt->m_job = ATTACK_QUEEN;
			}
			else
			{
				newAnt->m_job = DEFEND_QUEEN;
			}*/

			newAnt->m_job = DEFEND_QUEEN;
		}
		break;

		case AGENT_TYPE_SCOUT:
		{
			++m_numScouts;
			newAnt->m_job = SCOUT;
		}
		break;
	}

	++m_teamSize;
}


//-----------------------------------------------------------------------------------------------
bool Colony::SpawnNewAnt( Ant& queen )
{
	if ( queen.m_type != AGENT_TYPE_QUEEN 
		 || m_teamSize == m_startupInfo.matchInfo.colonyMaxPopulation )
	{
		return false;
	}
	
	int maxQueens = 4  + ( 2 * m_startupInfo.matchInfo.numPlayers / 2 );
	int maxWorkers = 30 + 20 * m_startupInfo.matchInfo.numPlayers;
	int maxSoldiers = 10 * ( m_startupInfo.matchInfo.numPlayers - 1 );
	int maxScouts = 5 * m_startupInfo.matchInfo.numPlayers;

	int upkeepCost = m_startupInfo.matchInfo.agentTypeInfos[AGENT_TYPE_QUEEN].upkeepPerTurn * m_numQueens
		+ m_startupInfo.matchInfo.agentTypeInfos[AGENT_TYPE_WORKER].upkeepPerTurn * m_numWorkers
		+ m_startupInfo.matchInfo.agentTypeInfos[AGENT_TYPE_SOLDIER].upkeepPerTurn * m_numSoldiers
		+ m_startupInfo.matchInfo.agentTypeInfos[AGENT_TYPE_SCOUT].upkeepPerTurn * m_numScouts;


	// Birth new workers if they'll take under 5% of total resources
	float percentCost		 = (float)( upkeepCost + m_startupInfo.matchInfo.agentTypeInfos[AGENT_TYPE_WORKER].costToBirth )	/ (float)m_nutrients;
	float percentCostQueen	 = (float)( upkeepCost + m_startupInfo.matchInfo.agentTypeInfos[AGENT_TYPE_QUEEN].costToBirth )		/ (float)m_nutrients;
	float percentCostSoldier = (float)( upkeepCost + m_startupInfo.matchInfo.agentTypeInfos[AGENT_TYPE_SOLDIER].costToBirth )	/ (float)m_nutrients;
	float percentCostScout   = (float)( upkeepCost + m_startupInfo.matchInfo.agentTypeInfos[AGENT_TYPE_SCOUT].costToBirth )		/ (float)m_nutrients;


	if ( queen.m_receivedCombatDamage )
	{
		queen.m_nextOrder = ORDER_BIRTH_SOLDIER;
		return true;
	}

	if ( m_myQueens.size() == 1 )
	{
		if ( percentCostQueen < .9f )
		{
			queen.m_nextOrder = ORDER_BIRTH_QUEEN;
			g_debug->LogText( "Birthed new queen!\n" );
			return true;
		}
		else
		{
			return false;
		}
	}

	if ( percentCostQueen < .6f
		 && m_myQueens.size() < maxQueens )
	{
		queen.m_nextOrder = ORDER_BIRTH_QUEEN;
		g_debug->LogText( "Birthed new queen!\n" );	
		return true;
	}
	else if ( m_numWorkers < 30
			  && percentCost < .05f )
	{
		queen.m_nextOrder = ORDER_BIRTH_WORKER;
		g_debug->LogText( "Birthed new worker!\n" );
		return true;
	}
	else if ( m_numSoldiers < maxSoldiers
			  && percentCostSoldier < .05f )
	{
		queen.m_nextOrder = ORDER_BIRTH_SOLDIER;
		g_debug->LogText( "Birthed new soldier!\n" );
		return true;
	}
	else if ( m_numScouts < maxScouts
			  && percentCostScout < .05f )
	{
		queen.m_nextOrder = ORDER_BIRTH_SCOUT;
		g_debug->LogText( "Birthed new scout!\n" );
		return true;
	}
	else if ( m_numWorkers < maxWorkers
			  && percentCost < .05f )
	{
		queen.m_nextOrder = ORDER_BIRTH_WORKER;
		g_debug->LogText( "Birthed new worker!\n" );
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Colony::MoveTowardsGoal( Ant& ant )
{
	ant.m_nextOrder = ORDER_HOLD;

	if ( ant.m_job == COLLECT_FOOD )// !ant.m_pathToGoal.empty() )
	{
		PathToGoal( ant );
		return;
	}

	if ( ant.m_type == AGENT_TYPE_QUEEN )
	{
		MoveTowardsGoalQueen( ant );
		return;
	}
	else if ( ant.m_type == AGENT_TYPE_SOLDIER )
	{
		MoveTowardsGoalSoldier( ant );
		return;
	}

	++ant.m_turnsTryingToReachGoal;

	/*if ( ant.m_turnsTryingToReachGoal > 40 )
	{
		ResetAnt( ant );
		return;
	}*/

	TileCoords coordsToGoal = ant.m_goalPos - ant.m_pos;

	// Made it to goal
	if ( coordsToGoal == TileCoords( 0, 0 ) )
	{
		ant.m_recentTileCosts.clear();
		ant.m_turnsTryingToReachGoal = 0;

		 if ( ant.m_type != AGENT_TYPE_WORKER )
		 {
			ResetAnt( ant );
			return;
		}

		if ( ant.m_hasFood )
		{
			ant.m_nextOrder = ORDER_DROP_CARRIED_OBJECT;
			return;
		}
		else
		{
			ant.m_nextOrder = ORDER_PICK_UP_FOOD;
			return;
		}
	}

	if ( ant.m_type == AGENT_TYPE_WORKER
		 && m_observedTiles[GetTileIdxFromTileCoords( ant.m_pos )].type == TILE_TYPE_DIRT )
	{
		ant.m_nextOrder = ORDER_DIG_HERE;
		return;
	}

	// Keep moving to goal
	bool isTileSafe[4] =
	{
		IsTileSafeForAnt( ant.m_pos + TileCoords( 1, 0 ), ant ),	// E
		IsTileSafeForAnt( ant.m_pos - TileCoords( 1, 0 ), ant ),	// W
		IsTileSafeForAnt( ant.m_pos + TileCoords( 0, 1 ), ant ),	// N
		IsTileSafeForAnt( ant.m_pos - TileCoords( 0, 1 ), ant )		// S
	};

	int directionWeights[4] = 
	{ 
		-coordsToGoal.x,  // E
		coordsToGoal.x,	  // W
		-coordsToGoal.y,  // N
		coordsToGoal.y	  // S
	};
		
	// Adjust weight based on unsafe tiles and find minimum cost
	int minCost = 999999;
	short bestDirection = EAST;
	for ( short dirIdx = 0; dirIdx < 4; ++dirIdx )
	{
		if ( !isTileSafe[dirIdx] )
		{
			directionWeights[dirIdx] += 99999;
		}

		// Account for tiles this ant has walked on during current journey
		auto recentTileCost = ant.m_recentTileCosts.find( GetAdjacentTileIdx( ant.m_pos, (eCardinalDirections)dirIdx ) );
		if ( recentTileCost != ant.m_recentTileCosts.end() )
		{
			directionWeights[dirIdx] += recentTileCost->second;
		}

		if ( directionWeights[dirIdx] < minCost )
		{
			minCost = directionWeights[dirIdx];
			bestDirection = dirIdx;
		}
	}

	// Nowhere to go, give up and pick a new goal
	if ( minCost > 99999 )
	{
		ResetAnt( ant );
		return;
	}

	switch ( bestDirection )
	{
		case EAST:	ant.m_nextOrder = ORDER_MOVE_EAST;  break;
		case WEST:	ant.m_nextOrder = ORDER_MOVE_WEST;  break;
		case NORTH: ant.m_nextOrder = ORDER_MOVE_NORTH; break;
		case SOUTH: ant.m_nextOrder = ORDER_MOVE_SOUTH; break;
	}

	ant.m_recentTileCosts[GetAdjacentTileIdx( ant.m_pos, (eCardinalDirections)bestDirection )] += 1;
}


//-----------------------------------------------------------------------------------------------
void Colony::MoveTowardsGoalQueen( Ant& ant )
{
	if ( ant.m_exhaustion > 1 )
	{
		ant.m_recentTileCosts[GetTileIdxFromTileCoords( ant.m_pos )] += 9999999;
		return;
	}
	
	++ant.m_turnsTryingToReachGoal;
	
	if ( ant.m_turnsTryingToReachGoal > ant.m_maxTurnsTryingToReachGoal )
	{
		AbandonGoal( ant );
		return;
	}

	TileCoords coordsToGoal = ant.m_goalPos - ant.m_pos;

	// Made it to goal
	if ( coordsToGoal == TileCoords( 0, 0 ) )
	{
		ant.m_abandonedGoals.clear();
		ResetAnt( ant );
		return;
	}

	// Keep moving to goal
	bool isTileSafe[4] =
	{
		IsTileSafeForAnt( ant.m_pos + TileCoords( 1, 0 ), ant ),	// E
		IsTileSafeForAnt( ant.m_pos - TileCoords( 1, 0 ), ant ),	// W
		IsTileSafeForAnt( ant.m_pos + TileCoords( 0, 1 ), ant ),	// N
		IsTileSafeForAnt( ant.m_pos - TileCoords( 0, 1 ), ant )		// S
	};

	int directionWeights[4] =
	{
		-coordsToGoal.x,  // E
		coordsToGoal.x,	  // W
		-coordsToGoal.y,  // N
		coordsToGoal.y	  // S
	};

	// Adjust weight based on unsafe tiles and find minimum cost
	int minCost = 999999;
	short bestDirection = EAST;
	for ( short dirIdx = 0; dirIdx < 4; ++dirIdx )
	{
		if ( !isTileSafe[dirIdx] )
		{
			directionWeights[dirIdx] += 9999999;
		}

		// Account for tiles this ant has walked on during current journey
		auto recentTileCost = ant.m_recentTileCosts.find( GetAdjacentTileIdx( ant.m_pos, (eCardinalDirections)dirIdx ) );
		if ( recentTileCost != ant.m_recentTileCosts.end() )
		{
			directionWeights[dirIdx] += recentTileCost->second;
		}

		if ( directionWeights[dirIdx] < minCost )
		{
			minCost = directionWeights[dirIdx];
			bestDirection = dirIdx;
		}
	}

	// Nowhere to go, give up and pick a new goal
	if ( minCost > 99999 )
	{
		AbandonGoal( ant );
		return;
	}

	switch ( bestDirection )
	{
		case EAST:	ant.m_nextOrder = ORDER_MOVE_EAST;  break;
		case WEST:	ant.m_nextOrder = ORDER_MOVE_WEST;  break;
		case NORTH: ant.m_nextOrder = ORDER_MOVE_NORTH; break;
		case SOUTH: ant.m_nextOrder = ORDER_MOVE_SOUTH; break;
	}
}


//-----------------------------------------------------------------------------------------------
void Colony::MoveTowardsGoalSoldier( Ant& ant )
{
	if ( ant.m_exhaustion > 1 )
	{
		ant.m_recentTileCosts[GetTileIdxFromTileCoords( ant.m_pos )] += 9999999;
		return;
	}

	++ant.m_turnsTryingToReachGoal;

	if ( ant.m_turnsTryingToReachGoal > ant.m_maxTurnsTryingToReachGoal )
	{
		ResetAnt( ant );
		return;
	}

	TileCoords coordsToGoal = ant.m_goalPos - ant.m_pos;

	// Made it to goal
	if ( coordsToGoal == TileCoords( 0, 0 ) )
	{
		ResetAnt( ant );
		return;
	}

	// Keep moving to goal
	bool isTileSafe[4] =
	{
		IsTileSafeForAnt( ant.m_pos + TileCoords( 1, 0 ), ant ),	// E
		IsTileSafeForAnt( ant.m_pos - TileCoords( 1, 0 ), ant ),	// W
		IsTileSafeForAnt( ant.m_pos + TileCoords( 0, 1 ), ant ),	// N
		IsTileSafeForAnt( ant.m_pos - TileCoords( 0, 1 ), ant )		// S
	};

	int directionWeights[4] =
	{
		-coordsToGoal.x,  // E
		coordsToGoal.x,	  // W
		-coordsToGoal.y,  // N
		coordsToGoal.y	  // S
	};

	// Adjust weight based on unsafe tiles and find minimum cost
	int minCost = 999999;
	short bestDirection = EAST;
	for ( short dirIdx = 0; dirIdx < 4; ++dirIdx )
	{
		if ( !isTileSafe[dirIdx] )
		{
			directionWeights[dirIdx] += 9999999;
		}

		// Account for tiles this ant has walked on during current journey
		auto recentTileCost = ant.m_recentTileCosts.find( GetAdjacentTileIdx( ant.m_pos, (eCardinalDirections)dirIdx ) );
		if ( recentTileCost != ant.m_recentTileCosts.end() )
		{
			directionWeights[dirIdx] += recentTileCost->second;
		}

		if ( directionWeights[dirIdx] < minCost )
		{
			minCost = directionWeights[dirIdx];
			bestDirection = dirIdx;
		}
	}

	// Nowhere to go, give up and pick a new goal
	if ( minCost > 99999 )
	{
		ResetAnt( ant );
		return;
	}

	switch ( bestDirection )
	{
		case EAST:	ant.m_nextOrder = ORDER_MOVE_EAST;  break;
		case WEST:	ant.m_nextOrder = ORDER_MOVE_WEST;  break;
		case NORTH: ant.m_nextOrder = ORDER_MOVE_NORTH; break;
		case SOUTH: ant.m_nextOrder = ORDER_MOVE_SOUTH; break;
	}
}


//-----------------------------------------------------------------------------------------------
void Colony::PathToGoal( Ant& ant )
{
	ant.m_nextOrder = ORDER_HOLD;

	if ( ant.m_exhaustion > 0 || ant.m_waitingOnPath )
	{
		return;
	}

	if ( ant.m_type == AGENT_TYPE_QUEEN )
	{
		if ( ant.m_lastPos == ant.m_pos )
		{
			++ant.m_turnsInTheSamePlace;
		}

		if ( ant.m_turnsInTheSamePlace > 10 )
		{
			AbandonGoal( ant );
			return;
		}

		ant.m_lastPos = ant.m_pos;
	}

	TileCoords coordsToGoal = ant.m_goalPos - ant.m_pos;

	// Made it to goal
	if ( coordsToGoal == TileCoords( 0, 0 ) )
	{
		ant.m_nextPathStep = TILE_COORDS_INVALID;
		ant.m_pathToGoal.clear();
		ant.m_abandonedGoals.clear();

		if ( ant.m_type != AGENT_TYPE_WORKER )
		{
			ResetAnt( ant );
			return;
		}

		if ( ant.m_hasFood )
		{
			ant.m_nextOrder = ORDER_DROP_CARRIED_OBJECT;
			return;
		}
		else
		{
			ant.m_nextOrder = ORDER_PICK_UP_FOOD;
			return;
		}
	}

	if ( ant.m_pathToGoal.empty() )
	{
		AbandonGoal( ant );
		return;
	}
	
	if ( ant.m_type == AGENT_TYPE_WORKER
		 && m_observedTiles[GetTileIdxFromTileCoords( ant.m_pos )].type == TILE_TYPE_DIRT )
	{
		ant.m_nextOrder = ORDER_DIG_HERE;
		return;
	}

	// only move on if we've successfully hit the next step
 	if ( ant.m_pos == ant.m_nextPathStep )
	{
		ant.m_nextPathStep = ant.m_pathToGoal[ant.m_pathToGoal.size() - 1];
		ant.m_pathToGoal.pop_back();
	}
	//else
	//{
	//	return;
	//}

	if		( ant.m_nextPathStep.x > ant.m_pos.x ) { ant.m_nextOrder = ORDER_MOVE_EAST; }
	else if ( ant.m_nextPathStep.x < ant.m_pos.x ) { ant.m_nextOrder = ORDER_MOVE_WEST; }
	else if ( ant.m_nextPathStep.y > ant.m_pos.y ) { ant.m_nextOrder = ORDER_MOVE_NORTH; }
	else if ( ant.m_nextPathStep.y < ant.m_pos.y ) { ant.m_nextOrder = ORDER_MOVE_SOUTH; }
}


//-----------------------------------------------------------------------------------------------
void Colony::PickNewGoal( Ant& ant )
{
	switch ( ant.m_job )
	{
		case COLLECT_FOOD:  PickNewGoalCollectFood( ant ); break;
		case DEFEND_QUEEN:  PickNewGoalDefendQueen( ant ); break;
		case ATTACK_QUEEN:  PickNewGoalAttackQueen( ant ); break;
		case SCOUT:			PickNewGoalScout( ant ); break;
		default:			ant.m_goalPos = ant.m_pos; break;
	}

	ant.m_maxTurnsTryingToReachGoal = ( ant.m_goalPos - ant.m_pos ).GetTaxicabLength() * 2;
	ant.m_pathToGoal.clear();

	if ( ant.m_job == COLLECT_FOOD )
	{
		RequestPathToGoalForAnt( ant );
	}
	//PathToGoal( ant );
	MoveTowardsGoal( ant );
}


//-----------------------------------------------------------------------------------------------
void Colony::PickNewGoalCollectFood( Ant& ant )
{
	//std::vector<eTileType> validTileTypes;
	TileCoords newTarget = ant.m_pos;

	switch ( ant.m_type )
	{
		case AGENT_TYPE_WORKER:
		{
			std::vector<eTileType> validTileTypes = { TILE_TYPE_AIR, TILE_TYPE_CORPSE_BRIDGE, TILE_TYPE_DIRT };

			newTarget = FindNearestUnclaimedFood( ant, validTileTypes, 1 );

		}
		break;

		case AGENT_TYPE_QUEEN:
		{
			//validTileTypes = { TILE_TYPE_AIR, TILE_TYPE_CORPSE_BRIDGE };

			newTarget = FindNextQueenGoal( ant );
			ant.m_lastPos = ant.m_pos;
		}
		break;
	}

	if ( newTarget == TILE_COORDS_INVALID )
	{
		ant.m_goalPos = GetTileCoordsFromTileIdx( GetRandomValidTileIdx( ant ) );
	}
	else
	{
		int tileIdx = GetTileIdxFromTileCoords( newTarget );
		m_observedTiles[tileIdx].assignedAnt = &ant;
		ant.m_goalPos = newTarget;
	}
}


//-----------------------------------------------------------------------------------------------
void Colony::PickNewGoalDefendQueen( Ant& ant )
{
	if ( m_startupInfo.matchInfo.numPlayers == 1 )
	{
		ant.m_goalPos = GetTileCoordsFromTileIdx( GetRandomValidTileIdx( ant ) );
		//MoveTowardsGoal( ant );
		return;
	}

	TileCoords newTarget = GetNearestEnemyLocation( ant.m_pos, 10 );

	if ( newTarget == TILE_COORDS_INVALID )
	{
		newTarget = GetNearestQueenLocation( ant.m_pos );
	}

	if ( newTarget == TILE_COORDS_INVALID )
	{
		ant.m_goalPos = GetTileCoordsFromTileIdx( GetRandomValidTileIdx( ant ) );
	}
	else
	{
		int tileIdx = GetTileIdxFromTileCoords( newTarget );
		ant.m_goalPos = newTarget;
	}
}


//-----------------------------------------------------------------------------------------------
void Colony::PickNewGoalAttackQueen( Ant& ant )
{
	if ( m_startupInfo.matchInfo.numPlayers == 1 )
	{
		ant.m_goalPos = GetTileCoordsFromTileIdx( GetRandomValidTileIdx( ant ) );
		//MoveTowardsGoal( ant );
		return;
	}

	TileCoords newTarget = GetNearestEnemyQueenLocation( ant.m_pos );
	if ( newTarget == TILE_COORDS_INVALID )
	{
		newTarget = GetNearestQueenLocation( ant.m_pos );
	}

	if ( newTarget == TILE_COORDS_INVALID )
	{
		ant.m_goalPos = GetTileCoordsFromTileIdx( GetRandomValidTileIdx( ant ) );
	}
	else
	{
		int tileIdx = GetTileIdxFromTileCoords( newTarget );
		ant.m_goalPos = newTarget;
	}
}


//-----------------------------------------------------------------------------------------------
void Colony::PickNewGoalScout( Ant& ant )
{
	ant.m_goalPos = GetTileCoordsFromTileIdx( GetRandomValidTileIdx( ant ) );
}


//-----------------------------------------------------------------------------------------------
TileCoords Colony::FindNearestUnclaimedFood( const Ant& ant, const std::vector<eTileType> validTileTypes, int minDistThreshold )
{
	int closestFood = 999999;
	int closestFoodIdx = GetRandomValidTileIdx( ant );
	for ( int i = 0; i < MAX_ARENA_TILES; ++i )
	{
		if ( m_observedTiles[i].hasFood
			 && m_observedTiles[i].assignedAnt == nullptr )
		{
			// Only consider valid types
			for ( eTileType const& validType : validTileTypes )
			{
				if ( validType == m_observedTiles[i].type )
				{
					int distToFood = ( GetTileCoordsFromTileIdx( i ) - ant.m_pos ).GetTaxicabLength();
					if ( distToFood  < closestFood 
						 && distToFood >= minDistThreshold )
					{
						closestFood = distToFood;
						closestFoodIdx = i;
					}
					break;
				}
			}
		}
	}

	return GetTileCoordsFromTileIdx( closestFoodIdx );
}


//-----------------------------------------------------------------------------------------------
TileCoords Colony::FindNextQueenGoal( const Ant& ant )
{
	int closestFood = 999999;
	int closestFoodIdx = GetRandomValidTileIdx( ant );
	for ( int i = 0; i < MAX_ARENA_TILES; ++i )
	{
		if ( !m_observedTiles[i].hasFood
			 /* && m_observedTiles[i].assignedAnt == nullptr*/ )
		{
			continue;
		}

		// Only consider valid types
		if ( m_observedTiles[i].type != TILE_TYPE_AIR
				&& m_observedTiles[i].type != TILE_TYPE_CORPSE_BRIDGE )
		{
			continue;
		}

		// Ignore previously abandoned goals
		bool isAbandoned = false;
		for ( TileCoords const& abandonedCoords : ant.m_abandonedGoals )
		{
			if ( i == GetTileIdxFromTileCoords( abandonedCoords ) )
			{
				isAbandoned = true;
				//break;
			}
		}

		if ( isAbandoned )
		{
			continue;
		}

		int distToFood = ( GetTileCoordsFromTileIdx( i ) - ant.m_pos ).GetTaxicabLength();
		if ( distToFood  < closestFood )
		{
			closestFood = distToFood;
			closestFoodIdx = i;
		}
	}

	return GetTileCoordsFromTileIdx( closestFoodIdx );
}


//-----------------------------------------------------------------------------------------------
int Colony::GetRandomValidTileIdx( const Ant& ant ) const
{
	int tileIdx = rand() % ( m_startupInfo.matchInfo.mapWidth  * m_startupInfo.matchInfo.mapWidth );

	while ( !IsTileSafeForAnt( GetTileCoordsFromTileIdx( tileIdx ), ant ) )
	{
		tileIdx = rand() % ( m_startupInfo.matchInfo.mapWidth  * m_startupInfo.matchInfo.mapWidth );
	}

	return tileIdx;
}


//-----------------------------------------------------------------------------------------------
TileCoords Colony::GetNearestQueenLocation( const TileCoords& startTile )
{
	int idx = 0;
	int closest = 999999;
	for ( int queenIdx = 0; queenIdx < (int)m_myQueens.size(); ++queenIdx )
	{
		int distToFood = ( m_myQueens[queenIdx]->m_pos - startTile ).GetTaxicabLength();
		if ( distToFood  < closest )
		{
			closest = distToFood;
			idx = queenIdx;
		}
	}

	return m_myQueens[idx]->m_pos;
}


//-----------------------------------------------------------------------------------------------
TileCoords Colony::GetNearestEnemyLocation( const TileCoords& startTile, int maxDist )
{
	int closestEnemy = 999999;
	int closestEnemyIdx = -1;
	for ( int i = 0; i < MAX_ARENA_TILES; ++i )
	{
		if ( m_observedTiles[i].antsInTile.size() > 0 )
		{
			
			int distToEnemy = ( GetTileCoordsFromTileIdx( i ) - startTile ).GetTaxicabLength();
			if ( distToEnemy < closestEnemy
					&& distToEnemy <= maxDist )
			{
				closestEnemy = distToEnemy;
				closestEnemyIdx = i;
			}
		}
	}

	if ( closestEnemyIdx == -1 )
	{
		return TILE_COORDS_INVALID;
	}

	return GetTileCoordsFromTileIdx( closestEnemyIdx );
}


//-----------------------------------------------------------------------------------------------
TileCoords Colony::GetNearestStationaryQueenLocation( const TileCoords& startTile )
{
	int idx = 0;
	int closest = 999999;
	for ( int queenIdx = 0; queenIdx < (int)m_myQueens.size(); ++queenIdx )
	{
		if ( m_myQueens[queenIdx]->m_job != STATIONARY_QUEEN )
		{
			continue;
		}

		int distToFood = ( m_myQueens[queenIdx]->m_pos - startTile ).GetTaxicabLength();
		if ( distToFood < closest )
		{
			closest = distToFood;
			idx = queenIdx;
		}
	}

	if ( closest == 999999 )
	{
		Ant* targetQueen = GetRandomStationaryQueen();
		if ( targetQueen == nullptr )
		{
			return TILE_COORDS_INVALID;
		}

		return targetQueen->m_pos;
	}

	return m_myQueens[idx]->m_pos;
}


//-----------------------------------------------------------------------------------------------
TileCoords Colony::GetNearestEnemyQueenLocation( const TileCoords& startTile )
{
	AgentID queenId = 0;
	int closest = 999999;
	for ( auto const& enemyQueen : m_enemyQueensByID )
	{
		int distToFood = ( enemyQueen.second - startTile ).GetTaxicabLength();
		if ( distToFood  < closest )
		{
			closest = distToFood;
			queenId = enemyQueen.first;
		}
	}

	return m_enemyQueensByID[queenId];
}


//-----------------------------------------------------------------------------------------------
Ant* Colony::GetRandomQueen()
{
	if ( m_myQueens.size() == 0 )
	{
		return nullptr;
	}

	if ( m_myQueens.size() == 1 )
	{
		return m_myQueens[0];
	}

	int queenIdx = rand() % (int)m_myQueens.size();
	return m_myQueens[queenIdx];
}


//-----------------------------------------------------------------------------------------------
Ant* Colony::GetRandomStationaryQueen()
{
	if ( m_myQueens.size() == 0 )
	{
		return nullptr;
	}

	if ( m_myQueens.size() == 1 )
	{
		return m_myQueens[0];
	}

	int queenIdx = rand() % (int)m_myQueens.size();
	int maxIter = 10;
	int numIter = 0;
	while ( numIter < maxIter
			&& m_myQueens[queenIdx]->m_job != STATIONARY_QUEEN )
	{
		queenIdx = rand() % (int)m_myQueens.size();
		++numIter;
	}

	return m_myQueens[queenIdx];
}


//-----------------------------------------------------------------------------------------------
void Colony::AbandonGoal( Ant& ant )
{
	ant.m_abandonedGoals.push_back( ant.m_goalPos );

	ResetAnt( ant );
}


//-----------------------------------------------------------------------------------------------
void Colony::ResetAnt( Ant& ant )
{
	ant.m_goalPos = TILE_COORDS_INVALID;
	ant.m_recentTileCosts.clear();
	ant.m_turnsTryingToReachGoal = 0;
	ant.m_pathToGoal.clear();
	ant.m_nextPathStep = TILE_COORDS_INVALID;
	ant.m_lastPos = TILE_COORDS_INVALID;
	ant.m_waitingOnPath = false;
	ant.m_turnsInTheSamePlace = 0;
	//if ( m_observedTiles[GetTileIdxFromTileCoords( ant.m_goalPos )].assignedAnt == &ant )
	//{
		//m_observedTiles[GetTileIdxFromTileCoords( ant.m_goalPos )].assignedAnt = nullptr;
	//}
}


//-----------------------------------------------------------------------------------------------
TileCoords Colony::GetTileCoordsFromTileIdx( int tileIdx ) const
{
	return ::GetTileCoordsFromTileIdx( tileIdx, m_startupInfo.matchInfo.mapWidth );
}


//-----------------------------------------------------------------------------------------------
int Colony::GetTileIdxFromTileCoords( const TileCoords& tileCoords ) const
{
	return ::GetTileIdxFromTileCoords( tileCoords, m_startupInfo.matchInfo.mapWidth );
}


//-----------------------------------------------------------------------------------------------
TileCoords Colony::GetAdjacentTile( const TileCoords& tileCoords, eCardinalDirections direction ) const
{
	return ::GetAdjacentTile( tileCoords, direction, m_startupInfo.matchInfo.mapWidth );
}


//-----------------------------------------------------------------------------------------------
int Colony::GetAdjacentTileIdx( const TileCoords& tileCoords, eCardinalDirections direction ) const
{
	return ::GetAdjacentTileIdx( tileCoords, direction, m_startupInfo.matchInfo.mapWidth );
}


//-----------------------------------------------------------------------------------------------
int Colony::GetAdjacentTileIdx( int tileIdx, eCardinalDirections direction ) const
{
	if ( tileIdx < 0
		 || tileIdx >= MAX_ARENA_TILES
		 || ( tileIdx % ( m_startupInfo.matchInfo.mapWidth - 1 ) ) == 0 && direction == EAST
		 || tileIdx == 0 && direction == WEST
		 || ( tileIdx > MAX_ARENA_TILES - m_startupInfo.matchInfo.mapWidth && direction == NORTH )
		 || ( tileIdx < m_startupInfo.matchInfo.mapWidth && direction == SOUTH )  )
	{
		return -1;
	}

	switch ( direction )
	{
		case EAST:	return tileIdx + 1;
		case WEST:	return tileIdx - 1;
		case NORTH:	return tileIdx + m_startupInfo.matchInfo.mapWidth;
		case SOUTH:	return tileIdx - m_startupInfo.matchInfo.mapWidth;
	}

	return -1;
}


//-----------------------------------------------------------------------------------------------
bool Colony::IsTileSafeForAnt( const TileCoords& tileCoords, const Ant& ant ) const
{
	if ( tileCoords.x < 0 || tileCoords.x > m_startupInfo.matchInfo.mapWidth - 1
		 || tileCoords.y < 0 || tileCoords.y > m_startupInfo.matchInfo.mapWidth - 1 )
	{
		return false;
	}

	int tileIdx = GetTileIdxFromTileCoords( tileCoords );

	switch ( ant.m_type )
	{
		case AGENT_TYPE_WORKER:
		{
			if ( m_observedTiles[tileIdx].type == TILE_TYPE_STONE
				 || m_observedTiles[tileIdx].type == TILE_TYPE_WATER
				 || m_observedTiles[tileIdx].type == TILE_TYPE_UNSEEN )
			{
				return false;
			}
		}
		break;

		case AGENT_TYPE_SOLDIER:
		{
			if ( m_observedTiles[tileIdx].type == TILE_TYPE_STONE
				 || m_observedTiles[tileIdx].type == TILE_TYPE_DIRT
				 || m_observedTiles[tileIdx].type == TILE_TYPE_UNSEEN
				 || m_startupInfo.matchInfo.numPlayers > 1 && m_observedTiles[tileIdx].type == TILE_TYPE_WATER )
			{
				return false;
			}
		}
		break;

		case AGENT_TYPE_SCOUT:
		{
			if ( m_observedTiles[tileIdx].type == TILE_TYPE_STONE)
			{
				return false;
			}
		}
		break;

		case AGENT_TYPE_QUEEN:
		{
			if ( m_observedTiles[tileIdx].type == TILE_TYPE_STONE
				 || m_observedTiles[tileIdx].type == TILE_TYPE_WATER
				 || m_observedTiles[tileIdx].type == TILE_TYPE_DIRT
				 || m_observedTiles[tileIdx].type == TILE_TYPE_UNSEEN )
			{
				return false;
			}
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
void Colony::RequestPathToGoalForAnt( Ant& ant )
{
	ant.m_pathToGoal.clear();

	// Already at goal, nowhere to path to
	if ( ant.m_pos == ant.m_goalPos )
	{
		//ant.m_pathToGoal = newPath;
		return;
	}

	GeneratePathRequestData requestData;
	requestData.id = ant.m_id;
	requestData.startTile = ant.m_pos;
	requestData.endTile = ant.m_goalPos;
	memcpy( requestData.observedTileTypes, m_observedTileTypes, MAX_ARENA_TILES * sizeof(eTileType) );
	requestData.mapWidth = m_startupInfo.matchInfo.mapWidth;

	requestData.tileTypeCosts[TILE_TYPE_AIR] = 0;
	requestData.tileTypeCosts[TILE_TYPE_CORPSE_BRIDGE] = 0;
	requestData.tileTypeCosts[TILE_TYPE_STONE] = -1;
	requestData.tileTypeCosts[TILE_TYPE_WATER] = -1;

	switch ( ant.m_type )
	{
		case AGENT_TYPE_QUEEN:
		{
			requestData.tileTypeCosts[TILE_TYPE_DIRT] = -1;
		}
		break;

		case AGENT_TYPE_WORKER:
		{
			requestData.tileTypeCosts[TILE_TYPE_DIRT] = 1;
		}
		break;

		case AGENT_TYPE_SOLDIER:
		{
			requestData.tileTypeCosts[TILE_TYPE_DIRT] = -1;
		}
		break;

		case AGENT_TYPE_SCOUT:
		{
			requestData.tileTypeCosts[TILE_TYPE_DIRT] = 0;
		}
		break;
	}

	g_generatePathRequests.Push( requestData );
	ant.m_nextPathStep = ant.m_pos;
	ant.m_waitingOnPath = true;
	/*ant.m_pathToGoal = GeneratePath( ant.m_pos, ant.m_goalPos, &tileTypeCosts[0] );
	if ( ant.m_pathToGoal.size() > 0 )
	{
		ant.m_nextPathStep = ant.m_pathToGoal[ant.m_pathToGoal.size() - 1];
	}*/
}

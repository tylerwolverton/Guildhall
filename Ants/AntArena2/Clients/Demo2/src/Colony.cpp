#include "Colony.hpp"
#include "ThreadSafeStructures.hpp"


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
	}
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessNewArenaUpdate()
{
	g_threadSafe_turnInfo.CopyTo( m_turnInfo );

	ProcessGeneralTurnInfo();
	ProcessVisibleTilesAndFood();
	ProcessDeathReports();
	ProcessBirthReports();
	ProcessOtherReports();
}


//------------------------------------------------------------------------------------------------
void Colony::AssignBasicOrdersToAnts()
{
	// #ToDo: loop through every Ant in the colony and determine its best "obvious" (no/low-cpu) order.
	//  If you have a path, follow it.  If you have a goal but no path, move toward it, etc.
	for( std::map< AgentID, Ant*>::iterator antIter = m_myAntsByID.begin(); antIter != m_myAntsByID.end(); ++ antIter )
	{
		Ant* ant = antIter->second;
		if( !ant || ant->m_lastState == STATE_DEAD )
		{
			continue;
		}

		// #ToDo: write some actual function(s) to decide what ants should do (e.g. follow a path, pick up food, etc.)
		eOrderCode moveOrders[4] = { ORDER_MOVE_EAST, ORDER_MOVE_NORTH, ORDER_MOVE_WEST, ORDER_MOVE_SOUTH };
		ant->m_nextOrder = moveOrders[ rand() % 4 ];
	}

	UpdateOrdersFromAnts(); // copy orders of all ants to shared ant orders array
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
}


//------------------------------------------------------------------------------------------------
void Colony::ProcessDeathReports()
{
	// #ToDo

//	for( int reportIndex = 0; reportIndex < m_turnInfo.numReports; ++ reportIndex )
//	{
//		AgentReport& report = m_turnInfo.agentReports[ reportIndex ];
//		if( IsDeathReport( report.result ) )
//		{
//			Ant* ant = GetMyAntForID( report.agentID );
//			if( ant )
//			{
//				OnAntDied( ant );
//			}
//		}
//	}
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
	ant.m_lastResult = report.result;
	ant.m_lastState = report.state;
	ant.m_pos.Set( report.tileX, report.tileY );

	// #ToDo: maybe react here based on what happened to this ant...?
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
	newAnt->m_lastResult = report.result;
	newAnt->m_lastState = report.state;

	m_myAntsByID[ report.agentID ] = newAnt;
}


#pragma once
#include "Common.hpp"


//------------------------------------------------------------------------------------------------
class ThreadSafe_TurnStatus;
class ThreadSafe_TurnInfo;
class ThreadSafe_TurnOrders;


//------------------------------------------------------------------------------------------------
// Thread-safe globals
//
extern ThreadSafe_TurnStatus	g_threadSafe_turnStatus;
extern ThreadSafe_TurnInfo		g_threadSafe_turnInfo;
extern ThreadSafe_TurnOrders	g_threadSafe_turnOrders;


//////////////////////////////////////////////////////////////////////////////////////////////////
// ThreadSafe_TurnStatus
//////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------
enum eTurnStatus : uint8_t
{
	TURN_STATUS_WAITING_FOR_PREGAME_STARTUP,
	TURN_STATUS_WAITING_FOR_NEXT_UPDATE,
	TURN_STATUS_PROCESSING_UPDATE,
	TURN_STATUS_WORKING_ON_ORDERS,
	TURN_STATUS_ORDERS_READY,
	NUM_TURN_STATUSES
};
const char* GetNameForTurnStatus( eTurnStatus turnStatus );


//------------------------------------------------------------------------------------------------
class ThreadSafe_TurnStatus
{
public:
	void Assert( eTurnStatus shouldBeTurnState );
	void Set( eTurnStatus newState );
	eTurnStatus Get() const;

private:
	eTurnStatus			m_turnState = TURN_STATUS_WAITING_FOR_PREGAME_STARTUP;
	mutable std::mutex	m_mutex;
};


//////////////////////////////////////////////////////////////////////////////////////////////////
// ThreadSafe_TurnInfo
//////////////////////////////////////////////////////////////////////////////////////////////////
class ThreadSafe_TurnInfo
{
public:
	void CopyFrom( const ArenaTurnStateForPlayer& copyFrom );
	void CopyTo( ArenaTurnStateForPlayer& copyTo ) const;

private:
	ArenaTurnStateForPlayer		m_arenaState;
	mutable std::mutex			m_mutex;
};


//////////////////////////////////////////////////////////////////////////////////////////////////
// ThreadSafe_TurnOrders
//////////////////////////////////////////////////////////////////////////////////////////////////
class ThreadSafe_TurnOrders
{
public:
	void CopyFrom( const PlayerTurnOrders& copyFrom );
	void CopyTo( PlayerTurnOrders& copyTo ) const;

private:
	PlayerTurnOrders			m_turnOrders;
	mutable std::mutex			m_mutex;
};



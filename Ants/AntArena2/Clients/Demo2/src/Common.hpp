#pragma once
#include "../../../Arena/Code/Game/ArenaPlayerInterface.hpp" // Provided by game master
#include "TileCoords.hpp"
#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <atomic>


//------------------------------------------------------------------------------------------------
class ThreadSafe_TurnStatus;
class ThreadSafe_TurnInfo;
class ThreadSafe_TurnOrders;


//------------------------------------------------------------------------------------------------
// Global constants
//
constexpr PlayerID		INVALID_PLAYER_ID	= 0xFF;
constexpr TeamID		INVALID_TEAM_ID		= 0xFF;
constexpr AgentID		INVALID_AGENT_ID	= 0xFFFFFFFF;


//------------------------------------------------------------------------------------------------
// Thread-safe globals
//
extern DebugInterface*			g_debug;					// Server function interface, use from any thread
extern std::atomic<bool>		g_isExiting;				// Set to true only once; signals all threads to exit
extern std::atomic<int>			g_threadSafe_threadCount;	// How many threads this DLL was given by the Arena
//extern ThreadSafe_TurnStatus	g_threadSafe_turnStatus;	// Thread-safe wrapper around eTurnStatus
extern ThreadSafe_TurnInfo		g_threadSafe_turnInfo;		// Middle "hand-off" buffer of triple-buffering scheme
extern ThreadSafe_TurnOrders	g_threadSafe_turnOrders;	// Middle "hand-off" buffer of triple-buffering scheme
extern std::atomic<int>			g_threadSafe_turnNumberOfLatestUpdateReceived;
extern std::atomic<int>			g_threadSafe_turnNumberOfLatestTurnOrdersSent;


//------------------------------------------------------------------------------------------------
// Global utility functions
//
void	Errorf( const char* format, ... );
bool	IsBirthReport( eAgentOrderResult orderResult );		// Does the report herald the arrival of a new Ant we need to add internally?
bool	IsDeathReport( eAgentOrderResult orderResult );		// Is the report indicating that this Ant died (for whatever reason)?
bool	IsStupidityReport( eAgentOrderResult orderResult );	// Is the given orderResult something caused by our own stupidity / incorrectness?





#include "Main.hpp"
#include "Common.hpp"
#include "Colony.hpp"
#include "ThreadSafeStructures.hpp"
#include "SynchronizedNonBlockingQueue.hpp"
#include "PathGenerator.hpp"


//------------------------------------------------------------------------------------------------
// Thread-safe globals
//
DebugInterface*		g_debug = nullptr;				// Server function interface, use from any thread
std::atomic<bool>	g_isExiting = false;			// Set to true only once; signals all threads to exit
std::atomic<int>	g_threadSafe_threadCount = 0;	// How many threads this DLL was given by the Arena

SynchronizedNonBlockingQueue<GeneratePathRequestData> g_generatePathRequests;
SynchronizedNonBlockingQueue<PathCompleteData> g_completedPaths;


//------------------------------------------------------------------------------------------------
// Entry point for our primary thread and any number of secondary threads.
//
// The primary thread "owns" the Colony instance; only the primary thread can directly use
//	g_colony, whose methods and data are NOT considered thread-safe (and are not for use by
//	other secondary worker threads) UNLESS they are specifically named "threadSafe".
//
// Only secondary threads process async work jobs in order to ensure processing of turn information is not delayed on the primary thread.
//
void ThreadMain( bool isPrimaryThread )
{
	++ g_threadSafe_threadCount;
	while( !g_isExiting )
	{
		if( isPrimaryThread )
		{
			g_colony->Update();
		}
		else
		{
			DoAsyncWork();
		}
	}
	-- g_threadSafe_threadCount;
}


//------------------------------------------------------------------------------------------------
void DoAsyncWork()
{
	// Calculate paths and such here
	GeneratePathRequestData requestData;
	if ( g_generatePathRequests.Pop( requestData ) )
	{
		// TODO: Just pass request object
		Path newPath = PathGenerator::GeneratePath( requestData.startTile, requestData.endTile, requestData.tileTypeCosts, requestData.observedTileTypes, requestData.mapWidth );

		PathCompleteData completeData;
		completeData.id = requestData.id;
		completeData.pathToGoal = newPath;
		g_completedPaths.Push( completeData );
		
	}

	std::this_thread::yield();
}


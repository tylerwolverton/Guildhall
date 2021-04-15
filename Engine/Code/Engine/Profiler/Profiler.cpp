#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Time/Time.hpp"

#include <map>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct ProfilerNode
{
	//ProfilerNode* parent;

	//// children
	//ProfilerNode* last_child;
	//ProfilerNode* prev_sibling; // first_child, prev_sibling, next_sibling 

	//char label[32];

	// timing
	double cummulativeTime = 0.0;
	int entryCount = 0;

	//void AddChild( ProfilerNode* child )
	//{
	//	child->parent = this;
	//	/*child->prev_sibling = last_child;
	//	last_child = child;*/
	//}
};


//-----------------------------------------------------------------------------------------------
static bool s_isProfilerEnabled = false;
static std::vector<std::string> s_functionNames;
static std::map<std::string, ProfilerNode> s_profilerNodes;


//-----------------------------------------------------------------------------------------------
void ProfilerPush( char const* label )
{
	UNUSED( label );
	//ProfilerNode newNode;

	//newNode.parent = tActiveNode;

	//// setup now
	//newNode.start_time = GetCurrentTimeSeconds();
	//newNode.label = tag;

	//if ( tActiveNode != nullptr ) {
	//	tActiveNode->add_child( new_node );
	//}

	//// this is now my active node
	//tActiveNode = new_node;
}


//-----------------------------------------------------------------------------------------------
void ProfilerPop()
{
	
}


//-----------------------------------------------------------------------------------------------
void ProfilerBeginFrame( char const* label )
{
	UNUSED( label );
}


//-----------------------------------------------------------------------------------------------
void ProfilerEndFrame()
{

}

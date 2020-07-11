#include "Game/GameJobs.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
// TestJob
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
void TestJob::Execute()
{
	/*std::string msg = Stringf( "Executing job %i", m_id );
	g_devConsole->PrintString( msg, Rgba8::ORANGE );*/
}


//-----------------------------------------------------------------------------------------------
void TestJob::ClaimJobCallback()
{
	std::string msg = Stringf( "Claiming job %i", m_id );
	g_devConsole->PrintString( msg, Rgba8::PURPLE );
}

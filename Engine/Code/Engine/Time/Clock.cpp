#include "Engine/Time/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Time/Time.hpp"


#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

static Clock* s_masterClock = nullptr;


//-----------------------------------------------------------------------------------------------
Clock::Clock()
{
	m_parentClock = s_masterClock;
	s_masterClock->AddChild( this );
}


//-----------------------------------------------------------------------------------------------
Clock::Clock( Clock* parent )
{
	m_parentClock = parent;

	if ( parent != nullptr )
	{
		m_parentClock->AddChild( this );
	}
}


//-----------------------------------------------------------------------------------------------
Clock::~Clock()
{
	for ( int childClockIdx = 0; childClockIdx < (int)m_childClocks.size(); ++childClockIdx )
	{
		if ( m_childClocks[childClockIdx] == nullptr )
		{
			continue;
		}

		m_childClocks[childClockIdx]->SetParent( m_parentClock );
		if ( m_parentClock != nullptr )
		{
			m_parentClock->AddChild( m_childClocks[childClockIdx] );
		}
	}

	m_childClocks.clear();
	
	if ( m_parentClock != nullptr )
	{

		m_parentClock = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
void Clock::Update( double deltaSeconds )
{
	deltaSeconds = ClampMinMax( deltaSeconds, 0.0, m_maxFrameTime );

	double minDeltaDiff = m_minFrameTime - deltaSeconds;
	if ( minDeltaDiff > 0.f )
	{
		deltaSeconds = m_minFrameTime;
		Sleep( DWORD(minDeltaDiff * 1000.0) );
	}

	if ( m_isPaused )
	{
		deltaSeconds = 0.0;
	}
	else
	{
		deltaSeconds *= m_timeScale;
	}

	m_deltaTimeSeconds = deltaSeconds;
	m_totalElapsedSeconds += deltaSeconds;

	for ( int childClockIdx = 0; childClockIdx < (int)m_childClocks.size(); ++childClockIdx )
	{
		if ( m_childClocks[childClockIdx] != nullptr )
		{
			m_childClocks[childClockIdx]->Update( deltaSeconds );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Clock::Reset()
{
	m_totalElapsedSeconds = 0.0;
}


//-----------------------------------------------------------------------------------------------
void Clock::Pause()
{
	m_isPaused = true;
}


//-----------------------------------------------------------------------------------------------
void Clock::Resume()
{
	m_isPaused = false;
}


//-----------------------------------------------------------------------------------------------
void Clock::SetScale( double scale )
{
	m_timeScale = scale;
}


//-----------------------------------------------------------------------------------------------
void Clock::SetFrameLimits( double minFrameTime, double maxFrameTime )
{
	m_minFrameTime = minFrameTime;
	m_maxFrameTime = maxFrameTime;
}


//-----------------------------------------------------------------------------------------------
void Clock::MasterStartup()
{
	s_masterClock = new Clock( nullptr );
}


//-----------------------------------------------------------------------------------------------
void Clock::MasterShutdown()
{
	PTR_SAFE_DELETE( s_masterClock );
}


//-----------------------------------------------------------------------------------------------
void Clock::MasterBeginFrame()
{
	static double timeLastFrameStarted = GetCurrentTimeSeconds(); // Runs once only!	
	double timeThisFrameStarted = GetCurrentTimeSeconds();
	double deltaSeconds = timeThisFrameStarted - timeLastFrameStarted;
	timeLastFrameStarted = timeThisFrameStarted;

	s_masterClock->Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
Clock* Clock::GetMaster()
{
	return s_masterClock;
}


//-----------------------------------------------------------------------------------------------
void Clock::SetParent( Clock* clock )
{
	m_parentClock = clock;
}


//-----------------------------------------------------------------------------------------------
void Clock::AddChild( Clock* clock )
{
	m_childClocks.push_back( clock );
}


//-----------------------------------------------------------------------------------------------
void Clock::RemoveChild( Clock* clock )
{
	for ( int childClockIdx = 0; childClockIdx < (int)m_childClocks.size(); ++childClockIdx )
	{
		if ( m_childClocks[childClockIdx] == clock )
		{
			m_childClocks[childClockIdx] = nullptr;
		}
	}
}

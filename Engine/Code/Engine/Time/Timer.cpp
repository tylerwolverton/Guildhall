#include "Engine/Time/Timer.hpp"
#include "Engine/Time/Clock.hpp"


//-----------------------------------------------------------------------------------------------
Timer::Timer( Clock* clock )
	: m_clock( clock )
{
	if ( m_clock == nullptr )
	{
		m_clock = Clock::GetMaster();
	}
}


//-----------------------------------------------------------------------------------------------
Timer::~Timer()
{

}


//-----------------------------------------------------------------------------------------------
void Timer::SetSeconds( Clock* clock, double timeToWait )
{
	m_clock = clock;
	m_startSeconds = clock->GetTotalElapsedSeconds();
	m_durationSeconds = timeToWait;
}


//-----------------------------------------------------------------------------------------------
void Timer::SetSeconds( double timeToWait )
{
	m_durationSeconds = timeToWait;
}


//-----------------------------------------------------------------------------------------------
void Timer::Reset()
{
	m_startSeconds = m_clock->GetTotalElapsedSeconds();
}


//-----------------------------------------------------------------------------------------------
void Timer::Stop()
{
	m_durationSeconds = -1.0;
}


//-----------------------------------------------------------------------------------------------
double Timer::GetElapsedSeconds() const
{
	double currentTime = m_clock->GetTotalElapsedSeconds();
	return ( currentTime - m_startSeconds );
}


//-----------------------------------------------------------------------------------------------
double Timer::GetSecondsRemaining() const
{
	return m_durationSeconds - GetElapsedSeconds();
}


//-----------------------------------------------------------------------------------------------
float Timer::GetRatioOfCompletion() const
{
	if ( m_durationSeconds == 0.0 )
	{
		return 1.f;
	}

	double elapsedSeconds = GetElapsedSeconds();
	return (float)( elapsedSeconds / m_durationSeconds );
}


//-----------------------------------------------------------------------------------------------
bool Timer::HasElapsed() const
{
	double currentTime = m_clock->GetTotalElapsedSeconds();
	return ( currentTime >= ( m_startSeconds + m_durationSeconds ) );
}


//-----------------------------------------------------------------------------------------------
bool Timer::CheckAndDecrement()
{
	if ( HasElapsed() ) 
	{
		// remove an interval of time
		m_startSeconds += m_durationSeconds;
		return true;
	}
	else 
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
int Timer::CheckAndDecrementAll()
{
	int numIncrements = 0;
	while ( CheckAndDecrement() )
	{
		++numIncrements;
	}

	return numIncrements;
}


//-----------------------------------------------------------------------------------------------
bool Timer::CheckAndReset()
{
	if ( HasElapsed() ) 
	{
		// remove an interval of time
		m_startSeconds = m_clock->GetTotalElapsedSeconds();
		return true;
	}
	else 
	{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
bool Timer::IsRunning() const
{
	return m_durationSeconds > 0.0;
}

#pragma once
#include <vector>


//-----------------------------------------------------------------------------------------------
class Clock
{
public:
	Clock();                   // defaults to being owned by master
	Clock( Clock* parent );    // passing nullptr will create a root clock (root clock is something you'd have to advance yourself)
	~Clock();                  // be sure to correctly re-parent my children to my parent

	void Update( double deltaSeconds );		 // usually do not need to call unless you create a new root clock
	void Reset();							 // set total time back to 0.0, does not reset children

	// Controls
	void Pause();
	void Resume();
	void SetScale( double scale );
	void SetFrameLimits( double minFrameTime, double maxFrameTime );

	// Accessors
	double GetTotalElapsedSeconds() const							{ return m_totalElapsedSeconds; }
	double GetLastDeltaSeconds() const								{ return m_deltaTimeSeconds; }

	double GetScale() const											{ return m_timeScale; }
	bool IsPaused() const											{ return m_isPaused; }

	// OPTIONAL - but useful
	// void SetFrameLimits( double minFrameTime, double maxFrameTime ); // useful if you never want a simluation jumping too far (for exapmle, while stuck on a breakpoint)
	
	// accessor for the master clock of our engine
	static void MasterStartup();  // create/reset master clock
	static void MasterShutdown();
	static void MasterBeginFrame();     // advance master clock (which immediately propagates to children)

	static Clock* GetMaster();

public:
	// used more internally
	void SetParent( Clock* clock );
	void AddChild( Clock* clock );
	void RemoveChild( Clock* clock );

private:
	//static Clock* s_masterClock;

	double m_totalElapsedSeconds = 0.0;
	double m_deltaTimeSeconds = 0.0;
	double m_minFrameTime = 0.0;
	double m_maxFrameTime = 0.1;

	bool m_isPaused = false;

	double m_timeScale = 1.0;

	Clock* m_parentClock = nullptr;
	std::vector<Clock*> m_childClocks;
};

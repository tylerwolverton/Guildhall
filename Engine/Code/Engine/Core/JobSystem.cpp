#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
// Job
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
Job::Job()
{
	static int s_nextJobId = 1;
	m_id = s_nextJobId++;
}


//-----------------------------------------------------------------------------------------------
// JobSystemWorkerThread
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
JobSystemWorkerThread::JobSystemWorkerThread()
{
	m_thread = new std::thread( &JobSystemWorkerThread::WorkerThreadMain, this );
}


//-----------------------------------------------------------------------------------------------
JobSystemWorkerThread::~JobSystemWorkerThread()
{
	m_thread->join();
	PTR_SAFE_DELETE( m_thread );
}


//-----------------------------------------------------------------------------------------------
void JobSystemWorkerThread::WorkerThreadMain()
{
	while ( !m_isQuitting )
	{
		Job* job = g_jobSystem->GetBestAvailableJob();
		if ( job != nullptr )
		{
			job->Execute();
			g_jobSystem->PostCompletedJob( job );
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
// JobSystem
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
JobSystem::~JobSystem()
{
	
}


//-----------------------------------------------------------------------------------------------
void JobSystem::Shutdown()
{
	StopAllThreads();

	PTR_VECTOR_SAFE_DELETE( m_workerThreads );
}


//-----------------------------------------------------------------------------------------------
void JobSystem::CreateWorkerThread()
{
	JobSystemWorkerThread* workerThread = new JobSystemWorkerThread();
	m_workerThreads.emplace_back( workerThread );
}


//-----------------------------------------------------------------------------------------------
void JobSystem::PostJob( Job* job )
{
	m_queuedJobsMutex.lock();
	m_queuedJobs.push_back( job );
	m_queuedJobsMutex.unlock();
}


//-----------------------------------------------------------------------------------------------
void JobSystem::PostCompletedJob( Job* job )
{
	m_completedJobsMutex.lock();
	m_completedJobs.push_back( job );
	m_completedJobsMutex.unlock();
}


//-----------------------------------------------------------------------------------------------
void JobSystem::ClaimAndDeleteAllCompletedJobs()
{
	std::deque<Job*> claimedJobs;

	m_completedJobsMutex.lock();
	m_completedJobs.swap( claimedJobs );
	m_completedJobsMutex.unlock();

	for ( int claimedJobIdx = 0; claimedJobIdx < (int)claimedJobs.size(); ++claimedJobIdx )
	{
		Job* claimedJob = claimedJobs[claimedJobIdx];
		claimedJob->ClaimJobCallback();
		PTR_SAFE_DELETE( claimedJob );
	}
}


//-----------------------------------------------------------------------------------------------
Job* JobSystem::GetBestAvailableJob()
{
	Job* job = nullptr;

	m_queuedJobsMutex.lock();
	if ( !m_queuedJobs.empty() )
	{
		job = m_queuedJobs.front();
		m_queuedJobs.pop_front();
	}
		
	m_queuedJobsMutex.unlock();

	return job;
}


//-----------------------------------------------------------------------------------------------
void JobSystem::StopAllThreads()
{
	for ( int workerIdx = 0; workerIdx < (int)m_workerThreads.size(); ++workerIdx )
	{
		m_workerThreads[workerIdx]->Quit();
	}
}


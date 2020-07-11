#pragma once
#include <mutex>
#include <thread>
#include <deque>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Job
{
public:
	Job();
	virtual ~Job() {}
	virtual void Execute() = 0;				// Called by worker thread
	virtual void ClaimJobCallback() {};		// Called by client on its thread

protected:
	int m_id = 0;
};


//-----------------------------------------------------------------------------------------------
class JobSystemWorkerThread
{
public:
	JobSystemWorkerThread();
	~JobSystemWorkerThread();

	void Quit()						{ m_isQuitting = true; }

private:
	void WorkerThreadMain();

private:
	std::thread* m_thread = nullptr;
	bool m_isQuitting = false;
};


//-----------------------------------------------------------------------------------------------
class JobSystem
{
public:
	JobSystem() {}
	~JobSystem();

	void Startup() {}
	void BeginFrame() {}
	void EndFrame() {}
	void Shutdown();

	void CreateWorkerThread();
	
	void PostJob( Job* job );
	void PostCompletedJob( Job* job );
	void ClaimAndDeleteAllCompletedJobs();

	Job* GetBestAvailableJob();

private:
	void StopAllThreads();

private:
	std::deque<Job*> m_queuedJobs;
	std::mutex m_queuedJobsMutex;
	std::deque<Job*> m_completedJobs;
	std::mutex m_completedJobsMutex;

	std::vector<JobSystemWorkerThread*> m_workerThreads;
};

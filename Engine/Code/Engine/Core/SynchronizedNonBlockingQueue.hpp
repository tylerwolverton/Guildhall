#pragma once
#include <queue>
#include <atomic>


//-----------------------------------------------------------------------------------------------
template<typename T>
class SynchronizedNonBlockingQueue
{

public:
	SynchronizedNonBlockingQueue() = default;
	SynchronizedNonBlockingQueue( SynchronizedNonBlockingQueue const& ) = delete;
	SynchronizedNonBlockingQueue( SynchronizedNonBlockingQueue&& ) = delete;
	~SynchronizedNonBlockingQueue() = default;

	SynchronizedNonBlockingQueue& operator=( SynchronizedNonBlockingQueue const& ) = delete;
	SynchronizedNonBlockingQueue& operator=( SynchronizedNonBlockingQueue const&& ) = delete;

	void Push( const T& value );
	bool Pop( T& out_value );
	
private:
	std::mutex		m_mutex;
	std::queue<T>	m_queue;
};


//-----------------------------------------------------------------------------------------------
template<typename T>
void SynchronizedNonBlockingQueue<T>::Push( const T& value )
{
	m_mutex.lock();
	m_queue.push( value );
	m_mutex.unlock();
}


//-----------------------------------------------------------------------------------------------
template<typename T>
typename bool SynchronizedNonBlockingQueue<T>::Pop( T& out_value )
{
	bool poppedSuccessfully = false;

	m_mutex.lock();

	if ( !m_queue.empty() )
	{
		out_value = m_queue.front();
		m_queue.pop();
		poppedSuccessfully = true;
	}

	m_mutex.unlock();

	return poppedSuccessfully;
}

#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>


//-----------------------------------------------------------------------------------------------
template<typename T>
class SynchronizedBlockingQueue : protected std::queue<T>
{
protected:
	typedef std::queue<T> base;

public:
	SynchronizedBlockingQueue() : base() {}
	SynchronizedBlockingQueue( SynchronizedBlockingQueue const& ) = delete;
	SynchronizedBlockingQueue( SynchronizedBlockingQueue const&& ) = delete;
	~SynchronizedBlockingQueue() = default;

	SynchronizedBlockingQueue& operator=( SynchronizedBlockingQueue const& ) = delete;
	SynchronizedBlockingQueue& operator=( SynchronizedBlockingQueue const&& ) = delete;

	void Push( const T& value );
	T Pop();

private:
	std::mutex m_lock;
	std::condition_variable m_condition;
};


//-----------------------------------------------------------------------------------------------
template<typename T>
void SynchronizedBlockingQueue<T>::Push( const T& value )
{
	std::lock_guard<std::mutex> guard( m_lock );
	base::push( value );
	m_condition.notify_all();
}


//-----------------------------------------------------------------------------------------------
template<typename T>
typename T SynchronizedBlockingQueue<T>::Pop()
{
	T value = nullptr;

	std::unique_lock<std::mutex> uniqueLock( m_lock );
	if ( base::empty() )
	{
		m_condition.wait( uniqueLock );
	}

	value = base::front();
	base::pop();

	return value;
}

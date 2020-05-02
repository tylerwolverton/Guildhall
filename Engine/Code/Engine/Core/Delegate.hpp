#pragma once
#include <vector>
#include <functional>


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
class Delegate
{
public:
	using function_t = std::function<void( ARGS... )>;
	using c_callback_t = void ( * )( ARGS... );

	//-----------------------------------------------------------------------------------------------
	struct Subscription
	{
		const void* objectId = nullptr;
		const void* functionId = nullptr;
		function_t callback;

		inline bool operator==( const Subscription& other ) const { return objectId == other.objectId && functionId == other.functionId; }
	};

public:
	void Subscribe( const c_callback_t& callback );
	void Unsubscribe( const c_callback_t& callback );

	template <typename OBJ_TYPE>
	void SubscribeMethod( OBJ_TYPE* obj, void ( OBJ_TYPE::*callbackMethod )( ARGS... ) );

	template <typename OBJ_TYPE>
	void UnsubscribeMethod( OBJ_TYPE* obj, void ( OBJ_TYPE::*callbackMethod )( ARGS... ) );

	template <typename OBJ_TYPE>
	void UnsubscribeAllMethodsFromObject( OBJ_TYPE* obj );

	void Invoke( const ARGS& ...args );

	void operator() ( const ARGS& ...args )							{ Invoke( args... ); }

private:
	void Subscribe( const Subscription& sub );
	void Unsubscribe( const Subscription& sub );

private:
	std::vector<Subscription> m_subscriptions;
};


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
template <typename OBJ_TYPE>
void Delegate<ARGS...>::UnsubscribeAllMethodsFromObject( OBJ_TYPE* obj )
{
	std::vector<uint> subIdxsToDelete;
	for ( uint subIdx = 0; subIdx < m_subscriptions.size(); ++subIdx )
	{
		if ( m_subscriptions[subIdx].objectId == obj )
		{
			subIdxsToDelete.push_back( subIdx );
		}
	}

	for ( uint deletedSubIdx = subIdxsToDelete.size() - 1; deletedSubIdx >= 0; --deletedSubIdx )
	{
		m_subscriptions.erase( m_subscriptions.begin() + deletedSubIdx );
	}
}


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
void Delegate<ARGS...>::Subscribe( const c_callback_t& callback )
{
	Subscription sub;
	sub.functionId = callback;
	sub.callback = callback;

	Subscribe( sub );
}


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
void Delegate<ARGS...>::Unsubscribe( const c_callback_t& callback )
{
	Subscription sub;
	sub.functionId = callback;

	Unsubscribe( sub );
}


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
void Delegate<ARGS...>::Invoke( const ARGS& ...args )
{
	for ( Subscription& sub : m_subscriptions )
	{
		sub.callback( args... );
	}
}


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
template <typename OBJ_TYPE>
void Delegate<ARGS...>::SubscribeMethod( OBJ_TYPE* obj, void ( OBJ_TYPE::*callbackMethod )( ARGS... ) )
{
	Subscription sub;
	sub.objectId = obj;
	sub.functionId = *(const void**)& callbackMethod; // Need to get the address, not safe if I wanted to call this

	sub.callback = [=]( ARGS ...args ) { ( obj->*callbackMethod )( args... ); };

	Subscribe( sub );
}


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
template <typename OBJ_TYPE>
void Delegate<ARGS...>::UnsubscribeMethod( OBJ_TYPE* obj, void ( OBJ_TYPE::* callbackMethod )( ARGS... ) )
{
	Subscription sub;
	sub.objectId = obj;
	sub.functionId = *(const void**)& callbackMethod; // Need to get the address, not safe if I wanted to call this

	Unsubscribe( sub );
}


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
void Delegate<ARGS...>::Subscribe( const Subscription& sub )
{
	m_subscriptions.push_back( sub );
}


//-----------------------------------------------------------------------------------------------
template <typename ...ARGS>
void Delegate<ARGS...>::Unsubscribe( const Subscription& sub )
{
	for ( uint subIdx = 0; subIdx < m_subscriptions.size(); ++subIdx )
	{
		if ( m_subscriptions[subIdx] == sub )
		{
			m_subscriptions.erase( m_subscriptions.begin() + subIdx );
			return;
		}
	}
}

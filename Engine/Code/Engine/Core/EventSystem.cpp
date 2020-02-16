#include "Engine/Core/EventSystem.hpp"


//-----------------------------------------------------------------------------------------------
EventSystem::EventSystem()
{
}


//-----------------------------------------------------------------------------------------------
EventSystem::~EventSystem()
{
}


//-----------------------------------------------------------------------------------------------
void EventSystem::Startup()
{
}


//-----------------------------------------------------------------------------------------------
void EventSystem::BeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
void EventSystem::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
void EventSystem::Shutdown()
{

}


//-----------------------------------------------------------------------------------------------
void EventSystem::RegisterEvent( std::string eventName, EventCallbackFunctionPtrType function )
{
	EventSubscription* newSubscription = new EventSubscription();
	newSubscription->m_eventName = eventName;
	newSubscription->m_callbackFuncPtr = function;

	m_eventSubscriptionPtrs.push_back( newSubscription );
}


//-----------------------------------------------------------------------------------------------
void EventSystem::DeRegisterEvent( std::string eventName, EventCallbackFunctionPtrType function )
{
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_eventSubscriptionPtrs.size(); ++subscriptionIndex )
	{
		if ( !_strcmpi( m_eventSubscriptionPtrs[subscriptionIndex]->m_eventName.c_str(), eventName.c_str() )
			 && m_eventSubscriptionPtrs[subscriptionIndex]->m_callbackFuncPtr == function )
		{
			m_eventSubscriptionPtrs[subscriptionIndex]->m_callbackFuncPtr = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void EventSystem::FireEvent( std::string eventName, const EventArgs* eventArgs )
{
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_eventSubscriptionPtrs.size(); ++subscriptionIndex )
	{
		if ( !_strcmpi( m_eventSubscriptionPtrs[subscriptionIndex]->m_eventName.c_str(), eventName.c_str() )
			 && m_eventSubscriptionPtrs[subscriptionIndex]->m_callbackFuncPtr != nullptr )
		{
			m_eventSubscriptionPtrs[subscriptionIndex]->m_callbackFuncPtr( *eventArgs );
		}
	}
}

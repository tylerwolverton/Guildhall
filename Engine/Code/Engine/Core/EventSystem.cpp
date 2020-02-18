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
void EventSystem::RegisterEvent( const std::string& eventName, const std::string& eventHelpText, eUsageLocation usageMode, EventCallbackFunctionPtrType function )
{
	EventSubscription* newSubscription = new EventSubscription();
	newSubscription->m_eventName = eventName;
	newSubscription->m_eventHelpText = eventHelpText;
	newSubscription->m_usageMode = usageMode;
	newSubscription->m_callbackFuncPtr = function;

	m_eventSubscriptionPtrs.push_back( newSubscription );
}


//-----------------------------------------------------------------------------------------------
void EventSystem::DeRegisterEvent( const std::string& eventName, EventCallbackFunctionPtrType function )
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
void EventSystem::FireEvent( const std::string& eventName, EventArgs* eventArgs, eUsageLocation location )
{
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_eventSubscriptionPtrs.size(); ++subscriptionIndex )
	{
		EventSubscription*& sub = m_eventSubscriptionPtrs[subscriptionIndex];
		if ( !_strcmpi( sub->m_eventName.c_str(), eventName.c_str() )
			 && sub->m_usageMode & location
			 && sub->m_callbackFuncPtr != nullptr )
		{
			sub->m_callbackFuncPtr( eventArgs );
		}
	}
}


//-----------------------------------------------------------------------------------------------
std::vector<EventSubscription*> EventSystem::GetAllExposedEventsForLocation( eUsageLocation location )
{
	std::vector<EventSubscription*> matchingEvents;
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_eventSubscriptionPtrs.size(); ++subscriptionIndex )
	{
		EventSubscription*& sub = m_eventSubscriptionPtrs[subscriptionIndex];
		if ( sub->m_usageMode & location )
		{
			matchingEvents.push_back( sub );
		}
	}

	return matchingEvents;
}

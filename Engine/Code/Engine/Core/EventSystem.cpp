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
void EventSystem::RegisterEvent( const std::string& eventName, const std::string& eventHelpText, eUsageLocation usageMode, Delegate<EventArgs*> function )
{
	DelegateEventSubscription newSub;
	newSub.m_eventName = eventName;
	newSub.m_eventHelpText = eventHelpText;
	newSub.m_usageMode = usageMode;
	newSub.m_delegate = function;

	m_delegateEventSubscriptions.push_back( newSub );
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
	EventArgs eventArgsObj;

	// Initialize event args if necessary and set this event's name into them
	if ( eventArgs == nullptr )
	{
		eventArgs = &eventArgsObj;
	}

	eventArgs->SetValue( "eventName", eventName );

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

	int numSubscriptions = (int)m_delegateEventSubscriptions.size();
	for ( int subscriptionIndex = 0; subscriptionIndex < numSubscriptions; ++subscriptionIndex )
	{
		DelegateEventSubscription& sub = m_delegateEventSubscriptions[subscriptionIndex];
		if ( !_strcmpi( sub.m_eventName.c_str(), eventName.c_str() )
			 && sub.m_usageMode & location )
		{
			sub.m_delegate.Invoke( eventArgs );
		}
	}
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> EventSystem::GetAllExposedEventNamesForLocation( eUsageLocation location )
{
	std::vector<std::string> matchingEvents;
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_eventSubscriptionPtrs.size(); ++subscriptionIndex )
	{
		EventSubscription*& sub = m_eventSubscriptionPtrs[subscriptionIndex];
		if ( sub->m_usageMode & location )
		{
			matchingEvents.push_back( sub->m_eventName );
		}
	}
	
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_delegateEventSubscriptions.size(); ++subscriptionIndex )
	{
		DelegateEventSubscription& sub = m_delegateEventSubscriptions[subscriptionIndex];
		if ( sub.m_usageMode & location )
		{
			matchingEvents.push_back( sub.m_eventName );
		}
	}

	return matchingEvents;
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> EventSystem::GetAllExposedEventHelpTextForLocation( eUsageLocation location )
{
	std::vector<std::string> matchingEvents;
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_eventSubscriptionPtrs.size(); ++subscriptionIndex )
	{
		EventSubscription*& sub = m_eventSubscriptionPtrs[subscriptionIndex];
		if ( sub->m_usageMode & location )
		{
			matchingEvents.push_back( sub->m_eventHelpText );
		}
	}

	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_delegateEventSubscriptions.size(); ++subscriptionIndex )
	{
		DelegateEventSubscription& sub = m_delegateEventSubscriptions[subscriptionIndex];
		if ( sub.m_usageMode & location )
		{
			matchingEvents.push_back( sub.m_eventHelpText );
		}
	}

	return matchingEvents;
}

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
void EventSystem::RegisterEvent( const std::string& eventName, const std::string& eventHelpText, eUsageLocation usageMode, EventCallbackFunctionPtrType function )
{
	EventSubscription* newSubscription = new EventSubscription();
	newSubscription->m_eventName = HashedString( eventName );
	newSubscription->m_eventHelpText = eventHelpText;
	newSubscription->m_usageMode = usageMode;
	newSubscription->m_callbackFuncPtr = function;

	m_eventSubscriptionPtrs.push_back( newSubscription );
}


//-----------------------------------------------------------------------------------------------
void EventSystem::RegisterEvent( const std::string& eventName, const std::string& eventHelpText, eUsageLocation usageMode, Delegate<EventArgs*> function )
{
	DelegateEventSubscription newSub;
	newSub.m_eventName = HashedString( eventName );
	newSub.m_eventHelpText = eventHelpText;
	newSub.m_usageMode = usageMode;
	newSub.m_delegate = function;

	m_delegateEventSubscriptions.push_back( newSub );
}


//-----------------------------------------------------------------------------------------------
void EventSystem::DeRegisterEvent( const std::string& eventName, EventCallbackFunctionPtrType function )
{
	HashedString hashedEventName( eventName );

	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_eventSubscriptionPtrs.size(); ++subscriptionIndex )
	{
		if ( m_eventSubscriptionPtrs[subscriptionIndex]->m_eventName == hashedEventName
			 && m_eventSubscriptionPtrs[subscriptionIndex]->m_callbackFuncPtr == function )
		{
			m_eventSubscriptionPtrs[subscriptionIndex]->m_callbackFuncPtr = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void EventSystem::FireEvent( const std::string& eventName, EventArgs* eventArgs, eUsageLocation location )
{
	HashedString hashedEventName( eventName );
	EventArgs eventArgsObj;

	// Initialize event args if necessary and set this event's name into them
	if ( eventArgs == nullptr )
	{
		eventArgs = &eventArgsObj;
	}

	eventArgs->SetValue( "eventName", eventName );

	// Copy current number of event registrations and iterate over them to fire events
	// This effectively ignores any new events that are registered from other events

	int curEventSubsCount = (int)m_eventSubscriptionPtrs.size();
	int curDelegateSubsCount = (int)m_delegateEventSubscriptions.size();

	for ( int subscriptionIndex = 0; subscriptionIndex < curEventSubsCount; ++subscriptionIndex )
	{
		EventSubscription*& sub = m_eventSubscriptionPtrs[subscriptionIndex];
		if ( sub->m_eventName == hashedEventName
			 && sub->m_usageMode & location
			 && sub->m_callbackFuncPtr != nullptr )
		{
			sub->m_callbackFuncPtr( eventArgs );
		}
	}

	for ( int subscriptionIndex = 0; subscriptionIndex < curDelegateSubsCount; ++subscriptionIndex )
	{
		DelegateEventSubscription& sub = m_delegateEventSubscriptions[subscriptionIndex];
		if ( sub.m_eventName == hashedEventName
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
			matchingEvents.push_back( sub->m_eventName.GetRawString() );
		}
	}
	
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_delegateEventSubscriptions.size(); ++subscriptionIndex )
	{
		DelegateEventSubscription& sub = m_delegateEventSubscriptions[subscriptionIndex];
		if ( sub.m_usageMode & location )
		{
			matchingEvents.push_back( sub.m_eventName.GetRawString() );
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

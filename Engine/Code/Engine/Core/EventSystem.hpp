#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/NamedProperties.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
typedef bool ( *EventCallbackFunctionPtrType )( EventArgs* );


//-----------------------------------------------------------------------------------------------
enum eUsageLocation : uint
{
	GAME = BIT_FLAG( 1 ),
	DEV_CONSOLE = BIT_FLAG( 2 ),
	EVERYWHERE = GAME | DEV_CONSOLE
};


//-----------------------------------------------------------------------------------------------
struct EventSubscription
{
public:
	std::string m_eventName;
	std::string m_eventHelpText;
	eUsageLocation m_usageMode = EVERYWHERE;
	EventCallbackFunctionPtrType m_callbackFuncPtr = nullptr;

public:
	EventSubscription() = default;
	~EventSubscription() = default;
};


//-----------------------------------------------------------------------------------------------
struct DelegateEventSubscription
{
public:
	std::string m_eventName;
	std::string m_eventHelpText;
	eUsageLocation m_usageMode = EVERYWHERE;
	Delegate<EventArgs*> m_delegate;

public:
	DelegateEventSubscription() = default;
	~DelegateEventSubscription() = default;
};


//-----------------------------------------------------------------------------------------------
class EventSystem
{
public:
	EventSystem();
	~EventSystem();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	// Register static function
	void RegisterEvent( const std::string& eventName, 
						const std::string& eventHelpText, 
						eUsageLocation m_usageMode, 
						EventCallbackFunctionPtrType function );

	void DeRegisterEvent( const std::string& eventName, 
						  EventCallbackFunctionPtrType function );

	void FireEvent( const std::string& eventName, 
					EventArgs* eventArgs = nullptr, 
					eUsageLocation location = eUsageLocation::GAME );

	std::vector<std::string> GetAllExposedEventNamesForLocation( eUsageLocation location );
	std::vector<std::string> GetAllExposedEventHelpTextForLocation( eUsageLocation location );

	// Register object methods
	void RegisterEvent( const std::string& eventName,
						const std::string& eventHelpText,
						eUsageLocation usageMode,
						Delegate<EventArgs*> function );

	template <typename OBJ_TYPE>
	void RegisterMethodEvent( const std::string& eventName, 
							  const std::string& eventHelpText, 
							  eUsageLocation usageMode, 
							  OBJ_TYPE* obj, 
							  void( OBJ_TYPE::*callbackMethod )( EventArgs* args ) );

	template <typename OBJ_TYPE>
	void DeRegisterMethodEvent( const std::string& eventName, 
								OBJ_TYPE* obj, 
								void( OBJ_TYPE::*callbackMethod )( EventArgs* args ) );

	template <typename OBJ_TYPE>
	void DeRegisterObject( OBJ_TYPE* obj );

private:
	std::vector<EventSubscription*> m_eventSubscriptionPtrs;

	std::vector<DelegateEventSubscription> m_delegateEventSubscriptions;
};


//-----------------------------------------------------------------------------------------------
template <typename OBJ_TYPE>
void EventSystem::RegisterMethodEvent( const std::string& eventName, 
									   const std::string& eventHelpText, 
									   eUsageLocation usageMode, 
									   OBJ_TYPE* obj, 
									   void( OBJ_TYPE::*callbackMethod )( EventArgs* args ) )
{
	DelegateEventSubscription newSub;
	newSub.m_eventName = eventName;
	newSub.m_eventHelpText = eventHelpText;
	newSub.m_usageMode = usageMode;
	
	newSub.m_delegate.SubscribeMethod( obj, callbackMethod );

	m_delegateEventSubscriptions.push_back( newSub );
}


//-----------------------------------------------------------------------------------------------
template <typename OBJ_TYPE>
void EventSystem::DeRegisterMethodEvent( const std::string& eventName, 
										 OBJ_TYPE* obj, 
										 void( OBJ_TYPE::*callbackMethod )( EventArgs* args ) )
{
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_delegateEventSubscriptions.size(); ++subscriptionIndex )
	{
		DelegateEventSubscription& sub = m_delegateEventSubscriptions[subscriptionIndex];
		if ( !_strcmpi( sub.m_eventName.c_str(), eventName.c_str() ) )
		{
			sub.m_delegate.UnsubscribeMethod( obj, callbackMethod );
			if ( sub.m_delegate.GetSubscriptionCount() == 0 )
			{		
				m_delegateEventSubscriptions.erase( m_delegateEventSubscriptions.begin() + subscriptionIndex );
			}
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
template <typename OBJ_TYPE>
void EventSystem::DeRegisterObject( OBJ_TYPE* obj )
{
	for ( int subscriptionIndex = 0; subscriptionIndex < (int)m_delegateEventSubscriptions.size(); ++subscriptionIndex )
	{
		DelegateEventSubscription& sub = m_delegateEventSubscriptions[subscriptionIndex];
		
		sub.m_delegate.UnsubscribeAllMethodsFromObject( obj );
	}
}

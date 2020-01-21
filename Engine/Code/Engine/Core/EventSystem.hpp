#pragma once
#include "Engine/Core/NamedStrings.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
typedef bool ( *EventCallbackFunctionPtrType )( EventArgs );


//-----------------------------------------------------------------------------------------------
struct EventSubscription
{
public:
	std::string m_eventName;
	EventCallbackFunctionPtrType m_callbackFuncPtr = nullptr;

public:
	EventSubscription() = default;
	~EventSubscription() = default;
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

	void RegisterEvent( std::string eventName, EventCallbackFunctionPtrType function);
	void DeRegisterEvent( std::string eventName, EventCallbackFunctionPtrType function);
	void FireEvent( std::string eventName, const EventArgs* eventArgs = nullptr );

private:
	std::vector<EventSubscription*> m_eventSubscriptionPtrs;
};
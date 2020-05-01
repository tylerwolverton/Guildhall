#pragma once
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"

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
	eUsageLocation m_usageMode;
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

	void RegisterEvent( const std::string& eventName, const std::string& m_eventHelpText, eUsageLocation m_usageMode, EventCallbackFunctionPtrType function );
	void DeRegisterEvent( const std::string& eventName, EventCallbackFunctionPtrType function );
	void FireEvent( const std::string& eventName, EventArgs* eventArgs = nullptr, eUsageLocation location = eUsageLocation::GAME );
	std::vector<EventSubscription*> GetAllExposedEventsForLocation( eUsageLocation location );

private:
	std::vector<EventSubscription*> m_eventSubscriptionPtrs;
};
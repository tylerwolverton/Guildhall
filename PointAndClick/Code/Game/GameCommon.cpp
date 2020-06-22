#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
App* g_app = nullptr;						// Owned by Main_Windows.cpp
Window* g_window = nullptr;					// Owned by Main_Windows.cpp
RenderContext* g_renderer = nullptr;		// Owned by the App
InputSystem* g_inputSystem = nullptr;		// Owned by the App
AudioSystem* g_audioSystem = nullptr;		// Owned by the App
Game* g_game = nullptr;						// Owned by the App


//-----------------------------------------------------------------------------------------------
std::string GetEventNameForVerbState( eVerbState verbState )
{
	switch ( verbState )
	{
		case eVerbState::NONE: return "None";
		case eVerbState::PICKUP: return OnPickUpVerbEventName;
		case eVerbState::OPEN: return OnOpenVerbEventName;
		case eVerbState::CLOSE: return OnCloseVerbEventName;
		case eVerbState::TALK_TO: return OnTalkToVerbEventName;
		case eVerbState::GIVE_TO_SOURCE: return OnGiveToSourceVerbEventName;
		case eVerbState::GIVE_TO_DESTINATION: return OnGiveToDestinationVerbEventName;
		default: return "Unknown State";
	}
}


//-----------------------------------------------------------------------------------------------
std::string GetVerbStateAsString( eVerbState verbState )
{
	switch ( verbState )
	{
		case eVerbState::NONE: return "None";
		case eVerbState::PICKUP: return "PickUp";
		case eVerbState::OPEN: return "Open";
		case eVerbState::CLOSE: return "Close";
		case eVerbState::TALK_TO: return "TalkTo";
		case eVerbState::GIVE_TO_SOURCE: return "GiveToSource";
		case eVerbState::GIVE_TO_DESTINATION: return "GiveToDestination";
		default: return "Unknown State";
	}
}


//-----------------------------------------------------------------------------------------------
std::string GetDisplayNameForVerbState( eVerbState verbState )
{
	switch ( verbState )
	{
		case eVerbState::NONE: return "None";
		case eVerbState::PICKUP: return "Pick Up";
		case eVerbState::OPEN: return "Open";
		case eVerbState::CLOSE: return "Close";
		case eVerbState::TALK_TO: return "Talk To";
		case eVerbState::GIVE_TO_SOURCE: return "Give";
		case eVerbState::GIVE_TO_DESTINATION: return "to";
		default: return "Unknown";
	}
}


//-----------------------------------------------------------------------------------------------
eVerbState GetVerbStateFromString( const std::string& typeStr )
{
	if ( typeStr == "PickUp" ) { return eVerbState::PICKUP; }
	if ( typeStr == "Open" ) { return eVerbState::OPEN; }
	if ( typeStr == "Close" ) { return eVerbState::CLOSE; }
	if ( typeStr == "TalkTo" ) { return eVerbState::TALK_TO; }
	if ( typeStr == "GiveToSource" ) { return eVerbState::GIVE_TO_SOURCE; }
	if ( typeStr == "GiveToDestination" ) { return eVerbState::GIVE_TO_DESTINATION; }

	return eVerbState::NONE;
}

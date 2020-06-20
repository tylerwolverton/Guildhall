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
		/*case eVerbState::GIVE_TO_SOURCE: return "Give to source";
		case eVerbState::GIVE_TO_DESTINATION: return "Give to destination";*/
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
		case eVerbState::GIVE_TO_SOURCE: return "Give to source";
		case eVerbState::GIVE_TO_DESTINATION: return "Give to destination";
		default: return "Unknown State";
	}
}


//-----------------------------------------------------------------------------------------------
eVerbState GetVerbStateFromString( const std::string& typeStr )
{
	if ( typeStr == "PickUp" ) { return eVerbState::PICKUP; }
	if ( typeStr == "Open" ) { return eVerbState::OPEN; }
	if ( typeStr == "Close" ) { return eVerbState::CLOSE; }
	if ( typeStr == "TalkTo" ) { return eVerbState::TALK_TO; }

	return eVerbState::NONE;
}

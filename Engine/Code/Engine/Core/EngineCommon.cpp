#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"


#include <math.h>


//-----------------------------------------------------------------------------------------------
DevConsole*  g_devConsole = nullptr;
EventSystem* g_eventSystem = nullptr;
NamedStrings g_gameConfigBlackboard;

const float fSQRT_3_OVER_3 = sqrt( 3.f ) / 3.f;
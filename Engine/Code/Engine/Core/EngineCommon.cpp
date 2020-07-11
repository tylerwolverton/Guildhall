#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/NamedStrings.hpp"

#include <math.h>


//-----------------------------------------------------------------------------------------------
EventSystem* g_eventSystem = nullptr;
DevConsole*  g_devConsole = nullptr;
JobSystem*  g_jobSystem = nullptr;
NamedStrings g_gameConfigBlackboard;


//-----------------------------------------------------------------------------------------------
const float fSQRT_3_OVER_3 = sqrt( 3.f ) / 3.f;
const float fPI = 3.14159265f;

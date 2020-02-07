#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec2.hpp"

#include <string>
#include <vector>
#include <cmath>


//-----------------------------------------------------------------------------------------------
class EventSystem;
class DevConsole;
class NamedStrings;


//-----------------------------------------------------------------------------------------------
extern EventSystem* g_eventSystem;
extern DevConsole* g_devConsole;
extern NamedStrings g_gameConfigBlackboard;


//-----------------------------------------------------------------------------------------------
typedef std::vector<std::string> Strings;
typedef std::vector<int> Ints;
typedef std::vector<float> Floats;
typedef NamedStrings EventArgs;


//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);


//-----------------------------------------------------------------------------------------------
const Vec2 ALIGN_CENTERED		= Vec2( 0.5f, 0.5f );
const Vec2 ALIGN_CENTERED_LEFT	= Vec2( 0.f, 0.5f );
const Vec2 ALIGN_CENTERED_RIGHT = Vec2( 1.f, 0.5f );
const Vec2 ALIGN_BOTTOM_LEFT	= Vec2( 0.f, 0.f );
const Vec2 ALIGN_BOTTOM_CENTER	= Vec2( 0.5f, 0.f );
const Vec2 ALIGN_BOTTOM_RIGHT	= Vec2( 1.f, 0.f );
const Vec2 ALIGN_TOP_LEFT		= Vec2( 0.f, 1.f );
const Vec2 ALIGN_TOP_CENTER		= Vec2( 0.5f, 1.f );
const Vec2 ALIGN_TOP_RIGHT		= Vec2( 1.f, 1.f );

extern const float fSQRT_3_OVER_3;

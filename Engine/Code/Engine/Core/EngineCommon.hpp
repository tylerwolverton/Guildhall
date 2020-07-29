#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec2.hpp"

#include <string>
#include <vector>
#include <map>
#include <cmath>


//-----------------------------------------------------------------------------------------------
class DevConsole;
class EventSystem;
class JobSystem;
class NamedProperties;
class NamedStrings;


//-----------------------------------------------------------------------------------------------
extern DevConsole* g_devConsole;
extern EventSystem* g_eventSystem;
extern JobSystem* g_jobSystem;
extern NamedStrings g_gameConfigBlackboard;


//-----------------------------------------------------------------------------------------------
typedef std::vector<std::string> Strings;
typedef std::vector<int> Ints;
typedef std::vector<float> Floats;
typedef NamedProperties EventArgs;

typedef unsigned int uint;
typedef unsigned char byte;


//-----------------------------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);
#define BIT_FLAG(b) (1<<(b))
#define PTR_SAFE_DELETE(ptr) if( ptr != nullptr ) { delete ptr; ptr = nullptr; }
#define PTR_VECTOR_SAFE_DELETE(vector) for( int vectorIdx = 0; vectorIdx < (int)vector.size(); ++vectorIdx ) {	PTR_SAFE_DELETE( vector[vectorIdx] ); } vector.clear();
#define PTR_MAP_SAFE_DELETE(map) for( auto it = map.begin(); it != map.end(); ++it ) {	PTR_SAFE_DELETE( it->second ); } map.clear();

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
extern const float fPI;

constexpr float UI_DEBUG_LINE_THICKNESS = 2.f;

constexpr int MAX_LIGHTS = 8;

//-----------------------------------------------------------------------------------------------
enum eScreenEdgesBitField : unsigned int
{
	SCREEN_EDGE_NONE = 0,
	SCREEN_EDGE_RIGHT = ( 1 << 0 ),
	SCREEN_EDGE_LEFT = ( 1 << 1 ),
	SCREEN_EDGE_TOP = ( 1 << 2 ),
	SCREEN_EDGE_BOTTOM = ( 1 << 3 )
};

#include "Engine/ZephyrCore/ZephyrEngineAPI.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/ZephyrCore/ZephyrEntity.hpp"


//#define REGISTER_EVENT( eventName ) {\
//										m_registeredMethods.insert( #eventName );\
//										g_eventSystem->RegisterMethodEvent( #eventName, "", EVERYWHERE, this, &ZephyrEngineAPI::eventName );\
//									}

//-----------------------------------------------------------------------------------------------
ZephyrEngineAPI::ZephyrEngineAPI()
{
	//REGISTER_EVENT( ChangeZephyrScriptState );
	/*REGISTER_EVENT( PrintDebugText );
	REGISTER_EVENT( PrintDebugScreenText );
	REGISTER_EVENT( PrintToConsole );*/
}


//-----------------------------------------------------------------------------------------------
ZephyrEngineAPI::~ZephyrEngineAPI()
{
	m_registeredMethods.clear();
}


//-----------------------------------------------------------------------------------------------
bool ZephyrEngineAPI::IsMethodRegistered( const std::string& methodName )
{
	auto iter = m_registeredMethods.find( methodName );
	
	return iter != m_registeredMethods.end();
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the current State of the Zephyr script for the entity who called the event.
 *	Note: Called like ChangeState( newState ) in script as a built in function
 *
 * params:
 *	- targetState: the name of the Zephyr State to change to
 *		- Zephyr type: String
*/
//-----------------------------------------------------------------------------------------------
//void ZephyrEngineAPI::ChangeZephyrScriptState( EventArgs* args )
//{
//	std::string targetState = args->GetValue( "targetState", "" );
//	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
//
//	if ( entity != nullptr
//		 && !targetState.empty() )
//	{
//		entity->ChangeZephyrScriptState( targetState );
//	}
//}
//
//
////-----------------------------------------------------------------------------------------------
///**
// * Print debug world text at position of entity who called the event.
// *
// * params:
// *	- text: text to print
// *		- Zephyr type: String
// *	- duration: duration in seconds to display text
// *		- Zephyr type: Number
// *		- default: 0 ( single frame )
// *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
// *		- Zephyr type: String
// *		- default: "white"
//*/
////-----------------------------------------------------------------------------------------------
//void ZephyrEngineAPI::PrintDebugText( EventArgs* args )
//{
//	std::string text = args->GetValue( "text", "TestPrint" );
//	float duration = args->GetValue( "duration", 0.f );
//	ZephyrEntity* entity = (ZephyrEntity*)args->GetValue( "entity", (void*)nullptr );
//
//	std::string colorStr = args->GetValue( "color", "white" );
//
//	Rgba8 color = Rgba8::WHITE;
//	if ( colorStr == "white" )		{ color = Rgba8::WHITE; }
//	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
//	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
//	else if ( colorStr == "blue" )  { color = Rgba8::BLUE; }
//	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }
//
//	Mat44 textLocation;
//
//	if ( entity != nullptr )
//	{
//		textLocation.SetTranslation2D( entity->GetPosition() );
//	}
//	
//	DebugAddWorldText( textLocation, Vec2::HALF, color, color, duration, .1f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, text.c_str() );
//}
//
//
////-----------------------------------------------------------------------------------------------
///**
// * Print debug screen text at given position.
// *
// * params:
// *	- text: text to print
// *		- Zephyr type: String
// *	- duration: duration in seconds to display text
// *		- Zephyr type: Number
// *		- default: 0 ( single frame )
// *	- fontSize: height in pixels of font 
// *		- Zephyr type: Number 
// *		- default: 24
// *	- locationRatio: position of font on screen, x and y are between 0 and 1
// *		- Zephyr type: Vec2
// *		- default: ( 0, 0 )
// *	- padding: how much padding in pixels to add to text position
// *		- Zephyr type: Vec2
// *		- default: ( 0, 0 )
// *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
// *		- Zephyr type: String
// *		- default: "white"
//*/
////-----------------------------------------------------------------------------------------------
//void ZephyrEngineAPI::PrintDebugScreenText( EventArgs* args )
//{
//	std::string text = args->GetValue( "text", "" );
//	float duration = args->GetValue( "duration", 0.f );
//	float fontSize = args->GetValue( "fontSize", 24.f );
//	Vec2 locationRatio = args->GetValue( "locationRatio", Vec2::ZERO );
//	Vec2 padding = args->GetValue( "padding", Vec2::ZERO );
//
//	std::string colorStr = args->GetValue( "color", "white" );
//
//	Rgba8 color = Rgba8::WHITE;
//	if ( colorStr == "white" )		{ color = Rgba8::WHITE; }
//	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
//	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
//	else if ( colorStr == "blue" )	{ color = Rgba8::BLUE; }
//	else if ( colorStr == "black" )	{ color = Rgba8::BLACK; }
//	
//	DebugAddScreenText( Vec4( locationRatio, padding ), Vec2::ZERO, fontSize, color, color, duration, text.c_str() );
//}
//
//
////-----------------------------------------------------------------------------------------------
///**
// * Print text to dev console.
// *
// * params:
// *	- text: text to print
// *		- Zephyr type: String
// *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
// *		- Zephyr type: String
// *		- default: "white"
//*/
////-----------------------------------------------------------------------------------------------
//void ZephyrEngineAPI::PrintToConsole( EventArgs* args )
//{
//	std::string text = args->GetValue( "text", "" );
//	std::string colorStr = args->GetValue( "color", "white" );
//
//	Rgba8 color = Rgba8::WHITE;
//	if		( colorStr == "white" ) { color = Rgba8::WHITE; }
//	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
//	else if ( colorStr == "green" )	{ color = Rgba8::GREEN; }
//	else if ( colorStr == "blue" )	{ color = Rgba8::BLUE; }
//	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }
//	
//	g_devConsole->PrintString( text.c_str(), color );
//}

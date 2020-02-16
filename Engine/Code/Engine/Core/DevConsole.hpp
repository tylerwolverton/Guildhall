#pragma once
#include "Engine/Core/Rgba8.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vertex_PCU;
struct Vec2;
struct AABB2;
class EventSystem;
class InputSystem;
class RenderContext;
class BitmapFont;
class Camera;


//-----------------------------------------------------------------------------------------------
struct DevConsoleLogMessage
{
public:
	std::string m_message;
	Rgba8 m_color = Rgba8::WHITE;

public:
	explicit DevConsoleLogMessage( std::string message, const Rgba8& color )
		: m_message( message )
		, m_color( color )
	{}
};


//-----------------------------------------------------------------------------------------------
struct DevConsoleCommand
{
public:
	std::string m_name;
	std::string m_helpText;

public:
	explicit DevConsoleCommand( std::string name, std::string helpText )
		: m_name( name )
		, m_helpText( helpText )
	{}
};


//-----------------------------------------------------------------------------------------------
class DevConsole
{
public:
	DevConsole();
	~DevConsole();

	void Startup();
	void BeginFrame();
	void Update( float deltaSeconds );
	void EndFrame();
	void Shutdown();

	void SetRenderer( RenderContext* renderer );
	void SetInputSystem( InputSystem* inputSystem );
	void SetEventSystem( EventSystem* eventSystem );
	void SetBitmapFont( BitmapFont* font );

	void ProcessInput();

	void PrintString( const std::string& message, const Rgba8& textColor = Rgba8::WHITE );

	void Render( const Camera& camera, float lineHeight = .03f ) const;
	void Render( const AABB2& bounds, float lineHeight = .03f ) const;

	void ToggleOpenFull();
	void Close();
	bool IsOpen() const												{ return m_isOpen; }

	void MoveCursorPosition( int deltaCursorPosition );
	void InsertCharacterIntoCommand( std::string character );

	void MoveThroughCommandHistory( int deltaCommandHistoryPosition );

private:
	void InitializeSupportedCommands();

	bool ProcessCharTyped( unsigned char character );
	bool ProcessKeyCode( unsigned char keyCode );
	void UpdateCursorBlink( float deltaSeconds );

	void RenderBackground( const AABB2& bounds ) const;
	void AppendVertsForLatestLogMessages( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const;
	void AppendVertsForInputString( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const;
	void AppendVertsForCursor( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const;
	void AppendVertsForString( std::vector<Vertex_PCU>& vertices, std::string message, const Rgba8& textColor, const Vec2& startMins, float lineHeight, float cellAspect = .56f, float spacingFraction = .2f ) const;
	
	void ExecuteCommand();
	void ExecuteQuitCommand();
	void ExecuteHelpCommand();

private:
	RenderContext* m_renderer = nullptr;
	InputSystem* m_inputSystem = nullptr;
	EventSystem* m_eventSystem = nullptr;

	BitmapFont* m_bitmapFont = nullptr;

	bool m_isOpen = false;
	Camera* m_devConsoleCamera = nullptr;
	std::vector<DevConsoleLogMessage> m_logMessages;

	std::string m_currentCommandStr;
	int m_currentCursorPosition = 0;
	int m_currentCommandHistoryPos = 0;
	std::vector<std::string> m_commandHistory;

	std::vector<DevConsoleCommand> m_supportedCommands;

	Rgba8 m_cursorColor = Rgba8::WHITE;
	float m_curCursorSeconds = 0.f;
	float m_maxCursorBlinkStateSeconds = .5f;
};

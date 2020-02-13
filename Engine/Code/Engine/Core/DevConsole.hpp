#pragma once
#include "Engine/Core/Rgba8.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
class EventSystem;
class InputSystem;
class RenderContext;
class Camera;


//-----------------------------------------------------------------------------------------------
struct DevConsoleLogMessage
{
public:
	Rgba8 m_color = Rgba8::WHITE;
	std::string m_message;

public:
	explicit DevConsoleLogMessage( const Rgba8& color, std::string message )
		: m_color( color )
		, m_message( message )
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

	void SetInputSystem( InputSystem* inputSystem );
	void SetEventSystem( EventSystem* eventSystem );

	void ProcessInput();

	void PrintString( const Rgba8& textColor, const std::string& devConsolePrintString );
	void Render( RenderContext& renderer, const Camera& camera, float lineHeight ) const;
	void Render( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const;

	void ToggleOpenFull();
	void Close();
	bool IsOpen() const												{ return m_isOpen; }

	void MoveCursorPosition( int deltaCursorPosition );
	void InsertCharacterIntoCommand( std::string character );

	bool ProcessCharTyped( unsigned char character );

private:
	void BlinkCursor( float deltaSeconds );

	void RenderBackground( RenderContext& renderer, const AABB2& bounds ) const;
	void RenderLatestLogMessages( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const;
	void RenderInputString( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const;
	void RenderCursor( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const;

	void ExecuteCommand();

private:
	InputSystem* m_inputSystem = nullptr;

	bool m_isOpen = false;
	Camera* m_devConsoleCamera = nullptr;
	std::vector<DevConsoleLogMessage> m_logMessages;

	EventSystem* m_eventSystem = nullptr;
	std::string m_currentCommandStr;
	int m_currentCursorPosition = 0;
	std::vector<std::string> m_commandHistory;

	Rgba8 m_cursorColor = Rgba8::WHITE;
	float m_curCursorSeconds = 0.f;
	float m_maxCursorBlinkStateSeconds = .5f;
};

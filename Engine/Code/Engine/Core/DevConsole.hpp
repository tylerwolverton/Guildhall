#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vertex_PCU;
struct Vec2;
struct AABB2;
class InputSystem;
class RenderContext;
class BitmapFont;
class Camera;
enum eCursorMode : uint;


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
	void SetBitmapFont( BitmapFont* font );

	void ProcessInput();

	void PrintString( const std::string& message, const Rgba8& textColor = Rgba8::WHITE );

	void Render( float lineHeight = 30.f ) const;
	void Render( const Camera& camera, float lineHeight = 30.f ) const;
	void Render( const AABB2& bounds, float lineHeight = 30.f ) const;

	void ToggleOpenFull();
	void Close();
	bool IsOpen() const												{ return m_isOpen; }

	void MoveCursorPosition( int deltaCursorPosition, bool updateInputIndex = true );
	void SetCursorPosition( int newCursorPosition, bool updateInputIndex = true );
	void InsertCharacterIntoCommand( std::string character );

	void MoveThroughCommandHistory( int deltaCommandHistoryPosition );

	static bool ShowHelp( EventArgs* args );

private:
	void LoadPersistentHistory();
	void SavePersistentHistory();

	bool ProcessCharTyped( unsigned char character );
	void UpdateFromKeyboard();
	void UpdateCursorBlink( float deltaSeconds );

	void RenderBackground( const AABB2& bounds ) const;
	void AppendVertsForLatestLogMessages( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const;
	void AppendVertsForInputString( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const;
	void AppendVertsForCursor( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const;
	void AppendVertsForString( std::vector<Vertex_PCU>& vertices, std::string message, const Rgba8& textColor, const Vec2& startMins, float lineHeight, float cellAspect = .56f, float spacingFraction = .2f ) const;
	
	void AutoCompleteCommand( bool isReversed );
	void UpdateAutoCompleteIdx( bool isReversed, int numCommands );
	void ExecuteCommand();

	void SetCommandString( std::string newString );
	void PasteFromClipboard();

private:
	RenderContext* m_renderer = nullptr;
	InputSystem* m_inputSystem = nullptr;

	BitmapFont* m_bitmapFont = nullptr;

	bool m_isOpen = false;
	Camera* m_devConsoleCamera = nullptr;
	std::vector<DevConsoleLogMessage> m_logMessages;

	std::string m_currentCommandStr;
	int m_currentCursorPosition = 0;
	int m_currentCommandHistoryPos = 0;
	std::vector<std::string> m_commandHistory;

	bool m_isSelectingText = false;
	int m_latestInputStringPosition = 0;
	int m_currentAutoCompleteIdx = -1;
	int m_currentSelectionEndPosition = -1;

	Rgba8 m_cursorColor = Rgba8::WHITE;
	float m_curCursorSeconds = 0.f;
	float m_maxCursorBlinkStateSeconds = .5f;
};

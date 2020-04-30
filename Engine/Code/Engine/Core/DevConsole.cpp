#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>


//-----------------------------------------------------------------------------------------------
DevConsole::DevConsole()
{
}


//-----------------------------------------------------------------------------------------------
DevConsole::~DevConsole()
{
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Startup()
{
	m_clock = new Clock();
	m_cursorBlinkTimer = new Timer( m_clock );
	m_cursorBlinkTimer->SetSeconds( .5f );

	m_devConsoleCamera = new Camera();
	m_devConsoleCamera->SetColorTarget( nullptr );
	m_devConsoleCamera->SetOutputSize( Vec2( 1920.f, 1080.f ) );
	m_devConsoleCamera->SetProjectionOrthographic( 1080.f );

	g_eventSystem->RegisterEvent( "help", "Display help text for each supported dev console command.", eUsageLocation::DEV_CONSOLE, ShowHelp );

	LoadPersistentHistory();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::BeginFrame()
{

}


//-----------------------------------------------------------------------------------------------
void DevConsole::Update()
{
	if ( !m_isOpen )
	{
		return;
	}

	UpdateCursorBlink();

	ProcessInput();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::EndFrame()
{

}


//-----------------------------------------------------------------------------------------------
void DevConsole::Shutdown()
{
	PTR_SAFE_DELETE( m_devConsoleCamera );
	PTR_SAFE_DELETE( m_cursorBlinkTimer );
	PTR_SAFE_DELETE( m_clock );

	m_renderer = nullptr;

	SavePersistentHistory();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SetRenderer( RenderContext* renderer )
{
	m_renderer = renderer;
	m_devConsoleCamera->SetColorTarget( renderer->GetBackBuffer() );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SetInputSystem( InputSystem* inputSystem )
{
	m_inputSystem = inputSystem;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SetBitmapFont( BitmapFont* font )
{
	m_bitmapFont = font;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::ProcessInput()
{
	if ( m_inputSystem == nullptr )
	{
		PrintString( "No input system bound to dev console", Rgba8::RED );
		return;
	}

	char c;
	while ( m_inputSystem->PopCharacter( &c ) )
	{
		ProcessCharTyped( c );
	}

	UpdateFromKeyboard();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::PrintString( const std::string& message, const Rgba8& textColor )
{
	m_logMessages.push_back( DevConsoleLogMessage( message, textColor ) );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Render( float lineHeight ) const
{
	AABB2 bounds( m_devConsoleCamera->GetOrthoMin(), m_devConsoleCamera->GetOrthoMax() );
	Render( bounds, lineHeight );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Render( const Camera& camera, float lineHeight ) const
{
	AABB2 bounds( camera.GetOrthoMin(), camera.GetOrthoMax() );
	Render( bounds, lineHeight );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Render( const AABB2& bounds, float lineHeight ) const
{
	if ( !m_isOpen 
		 || m_renderer == nullptr )
	{
		return;
	}
	
	m_renderer->SetBlendMode( eBlendMode::ALPHA );

	m_renderer->BeginCamera( *m_devConsoleCamera );
	
	AABB2 logMessageBounds = bounds.GetBoxAtTop( .95f );
	AABB2 inputStringBounds = bounds.GetBoxAtBottom( .045f );
	AABB2 inputCursorBounds = bounds.GetBoxAtBottom( .005f );
	inputStringBounds.mins.y = inputCursorBounds.maxs.y;

	std::vector<Vertex_PCU> vertices;
	
	RenderBackground( bounds );
	AppendVertsForLatestLogMessages( vertices, logMessageBounds, lineHeight );
	AppendVertsForInputString( vertices, inputStringBounds, lineHeight );
	AppendVertsForCursor( vertices, inputCursorBounds, lineHeight );
	
	if ( m_bitmapFont != nullptr )
	{
		m_renderer->BindDiffuseTexture( m_bitmapFont->GetTexture() );
	}
	else
	{
		m_renderer->BindDiffuseTexture( nullptr );
	}
	m_renderer->DrawVertexArray( vertices );
	
	m_renderer->EndCamera( *m_devConsoleCamera );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::UpdateCursorBlink()
{
	if ( m_cursorBlinkTimer->CheckAndReset() )
	{
		if ( m_cursorColor == Rgba8::WHITE )
		{
			m_cursorColor = Rgba8::BLACK;
		}
		else
		{
			m_cursorColor = Rgba8::WHITE;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::RenderBackground( const AABB2& bounds ) const
{
	Rgba8 backgroundColor = Rgba8::BLACK;
	backgroundColor.a = 150;

	std::vector<Vertex_PCU> backgroundVertices;
	AppendVertsForAABB2D( backgroundVertices, bounds, backgroundColor );

	m_renderer->BindDiffuseTexture( nullptr );
	m_renderer->DrawVertexArray( backgroundVertices );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::AppendVertsForLatestLogMessages( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const
{
	// Show one more line than can technically fit in order to show a partial line if possible
	float boundsHeightInPixels = bounds.maxs.y - bounds.mins.y;
	int maxNumLinesToRender = (int)( ( boundsHeightInPixels / lineHeight ) + 1 );

	// Nothing to print
	if ( m_logMessages.size() == 0
		 || maxNumLinesToRender <= 0 )
	{
		return;
	}

	// Adjust lines to render if too many exist to fit on screen
	int numLinesToRender = maxNumLinesToRender;
	if ( (int)m_logMessages.size() < numLinesToRender )
	{
		numLinesToRender = (int)m_logMessages.size();
	}
	
	float curLineY = 1;
	int latestMessageIndex = (int)m_logMessages.size() - 1;

	for ( int logMessageIndexFromEnd = 0; logMessageIndexFromEnd < numLinesToRender; ++logMessageIndexFromEnd )
	{
		int logMessageIndex = latestMessageIndex - logMessageIndexFromEnd;
		const DevConsoleLogMessage& logMessage = m_logMessages[logMessageIndex];
		Vec2 textMins( bounds.mins.x, bounds.mins.y + ( curLineY * lineHeight ) );
		
		AppendVertsForString( vertices, logMessage.m_message, logMessage.m_color, textMins, lineHeight );
		
		++curLineY;
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::AppendVertsForInputString( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const
{
	float cellAspect = .56f;
	if ( m_bitmapFont != nullptr
		 && m_bitmapFont->GetTexture() != nullptr )
	{
		cellAspect = .5f;
	}

	Vec2 startMins = Vec2( bounds.mins.x, bounds.mins.y );
	
	AppendVertsForString( vertices, "> " + m_currentCommandStr, Rgba8::WHITE, startMins, lineHeight, cellAspect );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::AppendVertsForCursor( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const
{
	float cellAspect = .56f;
	if ( m_bitmapFont != nullptr
		 && m_bitmapFont->GetTexture() != nullptr )
	{
		cellAspect = .5f;
	}

	float cellWidth = cellAspect * lineHeight;
	Vec2 startMins = Vec2( bounds.mins.x, bounds.mins.y );
	float startCursorPosition = (float)m_currentCursorPosition + 2.f;
	startMins.x += ( startCursorPosition * cellWidth );
	
	AppendVertsForString( vertices, "_", m_cursorColor, startMins, lineHeight, cellAspect );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::AppendVertsForString( std::vector<Vertex_PCU>& vertices, std::string message, const Rgba8& textColor, const Vec2& startMins, float lineHeight, float cellAspect, float spacingFraction ) const
{
	if ( m_bitmapFont == nullptr
		 || m_bitmapFont->GetTexture() == nullptr )
	{
		AppendTextTriangles2D( vertices, message, startMins, lineHeight, textColor, cellAspect, spacingFraction );
	}
	else
	{
		m_bitmapFont->AppendVertsForText2D( vertices, startMins, lineHeight, message, textColor, .5f );
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::AutoCompleteCommand( bool isReversed )
{
	// Get all commands available to dev console
	std::vector<EventSubscription*> supportedCommands = g_eventSystem->GetAllExposedEventsForLocation( eUsageLocation::DEV_CONSOLE );
	int numSupportedCommands = (int)supportedCommands.size();

	if ( numSupportedCommands == 0 )
	{
		return;
	}
		
	// Get all commands that start with current substr
	std::vector<std::string> potentialMatches;
	std::string subStrToMatch;
	if ( m_latestInputStringPosition > 0 )
	{
		subStrToMatch = m_currentCommandStr.substr( 0, m_latestInputStringPosition );
	}

	for ( int commandIdx = 0; commandIdx < (int)supportedCommands.size(); ++commandIdx )
	{
		EventSubscription*& sub = supportedCommands[commandIdx];
		if ( !_strnicmp( m_currentCommandStr.c_str(), sub->m_eventName.c_str(), m_latestInputStringPosition ) )
		{
			potentialMatches.push_back( sub->m_eventName );
		}
	}

	int numMatches = (int)potentialMatches.size();
	if ( numMatches == 0 )
	{
		return;
	}

	// Sort commands alphabetically and select the command to set based on position in auto complete sublist
	std::sort( potentialMatches.begin(), potentialMatches.end() );

	UpdateAutoCompleteIdx( isReversed, numMatches );

	// Handle case where user typed in a substr char for char and then tabs to next command
	if ( potentialMatches[m_currentAutoCompleteIdx] == m_currentCommandStr
		 && numMatches > 1 )
	{
		UpdateAutoCompleteIdx( isReversed, numMatches );
	}

	SetCommandString( potentialMatches[m_currentAutoCompleteIdx] );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::UpdateAutoCompleteIdx( bool isReversed, int numCommands )
{
	if ( isReversed )
	{
		--m_currentAutoCompleteIdx;
		if ( m_currentAutoCompleteIdx < 0 )
		{
			m_currentAutoCompleteIdx = numCommands - 1;
		}
	}
	else
	{
		++m_currentAutoCompleteIdx;
		if ( m_currentAutoCompleteIdx >= numCommands )
		{
			m_currentAutoCompleteIdx = 0;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::ToggleOpenFull()
{
	if ( m_isOpen )
	{
		Close();
	}
	else
	{
		m_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );
		m_inputSystem->ResetAllKeys();
		m_isOpen = true;
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Close()
{
	m_isOpen = false;
	m_currentCommandStr.clear();
	SetCursorPosition( 0 );
	m_currentCommandHistoryPos = (int)m_commandHistory.size();
	m_inputSystem->PopMouseOptions();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::MoveCursorPosition( int deltaCursorPosition, bool updateInputIndex )
{
	if ( deltaCursorPosition == 0 )
	{
		return;
	}

	m_currentCursorPosition += deltaCursorPosition;

	if ( m_currentCursorPosition < 0 )
	{
		SetCursorPosition( 0, false );
	}
	else if ( m_currentCursorPosition > (int)m_currentCommandStr.size() )
	{
		SetCursorPosition( (int)m_currentCommandStr.size(), false );
	}

	if ( updateInputIndex )
	{
		m_latestInputStringPosition = m_currentCursorPosition;
		m_currentAutoCompleteIdx = -1;
	}

	m_cursorColor = Rgba8::WHITE;
	m_cursorBlinkTimer->Reset();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SetCursorPosition( int newCursorPosition, bool updateInputIndex )
{
	m_currentCursorPosition = newCursorPosition;

	if ( updateInputIndex )
	{
		m_latestInputStringPosition = newCursorPosition;
		m_currentAutoCompleteIdx = -1;
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::InsertCharacterIntoCommand( std::string character )
{
	std::string newChar( "" + character );
	m_currentCommandStr.insert( (size_t)m_currentCursorPosition, newChar );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::MoveThroughCommandHistory( int deltaCommandHistoryPosition )
{
	if ( deltaCommandHistoryPosition == 0 )
	{
		return;
	}

	m_currentCommandHistoryPos += deltaCommandHistoryPosition;

	if ( m_currentCommandHistoryPos < 0 )
	{
		m_currentCommandHistoryPos = 0;
	}
	else if ( m_currentCommandHistoryPos >= (int)m_commandHistory.size() )
	{
		m_currentCommandHistoryPos = (int)m_commandHistory.size() - 1;
		return;
	}

	m_currentCommandStr.clear();

	if ( m_commandHistory.size() > 0 )
	{
		SetCommandString( m_commandHistory[m_currentCommandHistoryPos] );
		m_latestInputStringPosition = m_currentCursorPosition;
		m_currentAutoCompleteIdx = -1;
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::LoadPersistentHistory()
{
	std::string command;
	std::ifstream persistentHistory;
	persistentHistory.open( "Data/DevConsole/History.txt", std::ios::in );

	if ( !persistentHistory.is_open() )
	{
		return;
	}

	while ( std::getline( persistentHistory, command ) )
	{
		m_commandHistory.push_back( command );
	}

	persistentHistory.close();

	m_currentCommandHistoryPos = (int)m_commandHistory.size();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SavePersistentHistory()
{
	std::ofstream persistentHistory;
	persistentHistory.open( "Data/DevConsole/History.txt", std::ios::out );
	if ( !persistentHistory.is_open() )
	{
		return;
	}

	for ( int commandHistoryIdx = 0; commandHistoryIdx < (int)m_commandHistory.size(); ++commandHistoryIdx )
	{
		persistentHistory << m_commandHistory[commandHistoryIdx] << "\n";
	}

	persistentHistory.close();
}


//-----------------------------------------------------------------------------------------------
bool DevConsole::ProcessCharTyped( unsigned char character )
{
	if ( !m_isOpen )
	{
		return false;
	}

	if ( character == '~'
		 || character == '`' )
	{
		m_currentCommandStr.clear();
		SetCursorPosition( 0 );

		return true;
	}

	if ( character == '\r' )
	{
		ExecuteCommand();

		m_currentCommandStr.clear();
		SetCursorPosition( 0);

		return true;
	}

	if ( character == '\t' )
	{
		bool isReversed = m_inputSystem->IsKeyPressed( KEY_SHIFT );
		AutoCompleteCommand( isReversed );

		return true;
	}

	if ( character == '\b' )
	{
		if ( m_currentCursorPosition > 0 )
		{
			int deletePos = m_currentCursorPosition - 1;
			m_currentCommandStr.erase( deletePos, 1 );
			MoveCursorPosition( -1 );
		}

		return true;
	}

	if ( character == CMD_PASTE )
	{
		PasteFromClipboard();

		return true;
	}

	m_currentCommandStr.insert( m_currentCursorPosition, 1, character );
	MoveCursorPosition( 1 );

	return true;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::UpdateFromKeyboard()
{
	if ( !m_isOpen )
	{
		return;
	}

	MoveCursorPosition( m_inputSystem->ConsumeAllKeyPresses( KEY_RIGHTARROW ), false );
	MoveCursorPosition( -m_inputSystem->ConsumeAllKeyPresses( KEY_LEFTARROW ), false );

	if ( m_inputSystem->WasKeyJustPressed( KEY_HOME ) )
	{
		/*if ( m_inputSystem->IsKeyPressed( KEY_LEFT_SHIFT )
			 || m_inputSystem->IsKeyPressed( KEY_RIGHT_SHIFT ) )
		{
			m_currentSelectionEndPosition = m_currentCursorPosition;
		}*/

		SetCursorPosition( 0 );
	}

	if ( m_inputSystem->WasKeyJustPressed( KEY_END ) )
	{
		/*if ( m_inputSystem->IsKeyPressed( KEY_LEFT_SHIFT )
			 || m_inputSystem->IsKeyPressed( KEY_RIGHT_SHIFT ) )
		{
			m_currentSelectionEndPosition = m_currentCursorPosition;
		}*/

		SetCursorPosition( (int)m_currentCommandStr.size() );
	}

	MoveThroughCommandHistory( -m_inputSystem->ConsumeAllKeyPresses( KEY_UPARROW ) );
	MoveThroughCommandHistory( m_inputSystem->ConsumeAllKeyPresses( KEY_DOWNARROW ) );

	int numTimesDeletePressed = m_inputSystem->ConsumeAllKeyPresses( KEY_DELETE );
	int maxNumCharactersAvailableToDelete = (int)m_currentCommandStr.size() - m_currentCursorPosition;
	if ( numTimesDeletePressed > 0 
		 && maxNumCharactersAvailableToDelete > 0 )
	{
		int numCharactersToDelete = numTimesDeletePressed < maxNumCharactersAvailableToDelete ? numTimesDeletePressed : maxNumCharactersAvailableToDelete;

		m_currentCommandStr.erase( m_currentCursorPosition, numCharactersToDelete );
		m_latestInputStringPosition = m_currentCursorPosition;
		m_currentAutoCompleteIdx = -1;
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::ExecuteCommand()
{
	PrintString( "> " + m_currentCommandStr, Rgba8::WHITE );
	if ( m_currentCommandStr.size() == 0 )
	{
		return;
	}

	if ( m_commandHistory.empty()
		|| m_commandHistory.back() != m_currentCommandStr )
	{
		m_commandHistory.push_back( m_currentCommandStr );
		m_currentCommandHistoryPos = (int)m_commandHistory.size();
	}

	std::vector<std::string> args = SplitStringOnDelimiter( m_currentCommandStr, ' ' );

	// Check for a match in supported commands with case insensitivity
	std::vector<EventSubscription*> supportedCommands = g_eventSystem->GetAllExposedEventsForLocation( eUsageLocation::DEV_CONSOLE );

	for ( int cmdIdx = 0; cmdIdx < (int)supportedCommands.size(); ++cmdIdx )
	{
		EventSubscription*& cmd = supportedCommands[cmdIdx];
		if ( cmd != nullptr 
			 && !_strcmpi( args[0].c_str(), cmd->m_eventName.c_str() ) )
		{
			EventArgs eventArgs;

			if ( (int)args.size() > 1 )
			{
				for ( int argIdx = 1; argIdx < (int)args.size(); ++argIdx )
				{
					// Change this as a method in named strings that tokenizes and parses commands to support quoted strings with =
					std::vector<std::string> params = SplitStringOnDelimiter( args[argIdx], '=' );
					if ( (int)params.size() == 2 )
					{
						eventArgs.SetValue( params[0], params[1] );
					}
				}
			}

			g_eventSystem->FireEvent( args[0], &eventArgs, eUsageLocation::DEV_CONSOLE );

			return;
		}
	}
	
	PrintString( "Invalid command: '" + m_currentCommandStr + "'", Rgba8::RED );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SetCommandString( std::string newString )
{
	m_currentCommandStr = newString;
	m_currentCursorPosition = (int)m_currentCommandStr.size();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::PasteFromClipboard()
{
	const char* clipboardText = m_inputSystem->GetTextFromClipboard();
	if ( clipboardText == nullptr )
	{
		return;
	}

	m_currentCommandStr.insert( m_currentCursorPosition, clipboardText );
	SetCursorPosition( (int)m_currentCommandStr.size() );
}


//-----------------------------------------------------------------------------------------------
bool DevConsole::ShowHelp( EventArgs* args )
{
	UNUSED( args );

	std::vector<EventSubscription*> supportedCommands = g_eventSystem->GetAllExposedEventsForLocation( eUsageLocation::DEV_CONSOLE );

	for ( int cmdIdx = 0; cmdIdx < (int)supportedCommands.size(); ++cmdIdx )
	{
		EventSubscription*& cmd = supportedCommands[cmdIdx];
		if ( cmd != nullptr )
		{
			g_devConsole->PrintString( cmd->m_eventName + " - " + cmd->m_eventHelpText, Rgba8::WHITE );
		}
	}

	return 0;
}

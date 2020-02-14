#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"


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
	m_devConsoleCamera = new Camera();
	m_devConsoleCamera->SetColorTarget( nullptr );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::BeginFrame()
{

}


//-----------------------------------------------------------------------------------------------
void DevConsole::Update( float deltaSeconds )
{
	if ( !m_isOpen )
	{
		return;
	}

	UpdateCursorBlink( deltaSeconds );

	ProcessInput();
	
	m_devConsoleCamera->SetOrthoView( Vec2( -1.f, -1.f ), Vec2( 1.f, 1.f ) );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::EndFrame()
{

}


//-----------------------------------------------------------------------------------------------
void DevConsole::Shutdown()
{
	delete m_devConsoleCamera;
	m_devConsoleCamera = nullptr;

	m_renderer = nullptr;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SetRenderer( RenderContext* renderer )
{
	m_renderer = renderer;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SetInputSystem( InputSystem* inputSystem )
{
	m_inputSystem = inputSystem;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::SetEventSystem( EventSystem* eventSystem )
{
	m_eventSystem = eventSystem;
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
}


//-----------------------------------------------------------------------------------------------
void DevConsole::PrintString( const std::string& message, const Rgba8& textColor )
{
	m_logMessages.push_back( DevConsoleLogMessage( message, textColor ) );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Render( const Camera& camera, float lineHeight ) const
{
	AABB2 bounds( camera.GetOrthoBottomLeft(), camera.GetOrthoTopRight() );
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

	m_renderer->BeginCamera( *m_devConsoleCamera );
	
	AABB2 logMessageBounds = bounds.GetBoxAtTop( .95f );
	AABB2 inputStringBounds = bounds.GetBoxAtBottom( .04f );
	AABB2 inputCarotBounds = bounds.GetBoxAtBottom( .01f );

	std::vector<Vertex_PCU> vertices;
	
	RenderBackground( bounds );
	AppendVertsForLatestLogMessages( vertices, logMessageBounds, lineHeight );
	AppendVertsForInputString( vertices, inputStringBounds, lineHeight );
	AppendVertsForCursor( vertices, inputCarotBounds, lineHeight );
	
	if ( m_bitmapFont != nullptr )
	{
		m_renderer->BindTexture( m_bitmapFont->GetTexture() );
	}
	else
	{
		m_renderer->BindTexture( nullptr );
	}
	m_renderer->DrawVertexArray( vertices );
	
	m_renderer->EndCamera( *m_devConsoleCamera );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::UpdateCursorBlink( float deltaSeconds )
{
	m_curCursorSeconds += deltaSeconds;

	if ( m_curCursorSeconds > m_maxCursorBlinkStateSeconds )
	{
		if ( m_cursorColor == Rgba8::WHITE )
		{
			m_cursorColor = Rgba8::BLACK;
		}
		else
		{
			m_cursorColor = Rgba8::WHITE;
		}

		m_curCursorSeconds = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::RenderBackground( const AABB2& bounds ) const
{
	Rgba8 backgroundColor = Rgba8::BLACK;
	backgroundColor.a = 150;

	std::vector<Vertex_PCU> backgroundVertices;
	m_renderer->AppendVertsForAABB2D( backgroundVertices, bounds, backgroundColor );

	m_renderer->BindTexture( nullptr );
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

		if ( m_bitmapFont == nullptr 
			|| m_bitmapFont->GetTexture() == nullptr )
		{
			AppendTextTriangles2D( vertices, logMessage.m_message, textMins, lineHeight, logMessage.m_color );
		}
		else
		{
			m_bitmapFont->AppendVertsForText2D( vertices, textMins, lineHeight, logMessage.m_message, logMessage.m_color );
		}
		
		++curLineY;
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::AppendVertsForInputString( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const
{
	float cellAspect = .56f;
	float cellWidth = cellAspect * lineHeight;
	float spacingFraction = .2f;
	Vec2 startMins = Vec2( bounds.mins.x, bounds.mins.y );
	
	if ( m_bitmapFont == nullptr
		 || m_bitmapFont->GetTexture() == nullptr )
	{
		AppendTextTriangles2D( vertices, ">", startMins, lineHeight, Rgba8::WHITE, cellAspect, spacingFraction );
	}
	else
	{
		m_bitmapFont->AppendVertsForText2D( vertices, startMins, lineHeight, ">", Rgba8::WHITE, cellAspect );
	}

	startMins.x += cellWidth + ( cellWidth * spacingFraction );

	if ( m_bitmapFont == nullptr
		 || m_bitmapFont->GetTexture() == nullptr )
	{
		AppendTextTriangles2D( vertices, m_currentCommandStr, startMins, lineHeight, Rgba8::WHITE, cellAspect, spacingFraction );
	}
	else
	{
		m_bitmapFont->AppendVertsForText2D( vertices, startMins, lineHeight, m_currentCommandStr, Rgba8::WHITE, cellAspect );
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::AppendVertsForCursor( std::vector<Vertex_PCU>& vertices, const AABB2& bounds, float lineHeight ) const
{
	float cellAspect = .56f;
	float cellWidth = cellAspect * lineHeight;
	float spacingFraction = .2f;
	Vec2 startMins = Vec2( bounds.mins.x, bounds.mins.y );

	float startCursorPosition = (float)m_currentCursorPosition + 1.f;
	startMins.x += ( startCursorPosition * cellWidth ) + .01f; //+ ( startCursorPosition * cellWidth * spacingFraction ) );
	
	if ( m_bitmapFont == nullptr
		 || m_bitmapFont->GetTexture() == nullptr )
	{
		AppendTextTriangles2D( vertices, "_", startMins, lineHeight, m_cursorColor, cellAspect, spacingFraction );
	}
	else
	{
		m_bitmapFont->AppendVertsForText2D( vertices, startMins, lineHeight, "_", m_cursorColor, cellAspect );
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
		m_isOpen = true;
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Close()
{
	m_isOpen = false;
	m_currentCommandHistoryPos = (int)m_commandHistory.size();
}


//-----------------------------------------------------------------------------------------------
void DevConsole::MoveCursorPosition( int deltaCursorPosition )
{
	m_currentCursorPosition += deltaCursorPosition;

	if ( m_currentCursorPosition < 0 )
	{
		m_currentCursorPosition = 0;
	}
	else if ( m_currentCursorPosition > (int)m_currentCommandStr.size() )
	{
		m_currentCursorPosition = (int)m_currentCommandStr.size();
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

	m_currentCommandStr = m_commandHistory[m_currentCommandHistoryPos];
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
		m_currentCursorPosition = 0;
		
		return true;
	}

	if ( character == '\r' )
	{
		ExecuteCommand();

		m_currentCommandStr.clear();
		m_currentCursorPosition = 0;

		return true;
	}

	if ( character == '\b' )
	{
		if ( m_currentCursorPosition > 0 )
		{
			int deletePos = m_currentCursorPosition - 1;
			m_currentCommandStr.erase( deletePos, 1 );
			--m_currentCursorPosition;
		}

		return true;
	}

	m_currentCommandStr += character;
	++m_currentCursorPosition;

	return true;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::ExecuteCommand()
{
	if ( m_eventSystem == nullptr )
	{
		return;
	}

	m_commandHistory.push_back( m_currentCommandStr );
	m_currentCommandHistoryPos = (int)m_commandHistory.size();

	PrintString( "> " + m_currentCommandStr, Rgba8::WHITE );

	EventArgs args;
	m_eventSystem->FireEvent( m_currentCommandStr, &args );
}

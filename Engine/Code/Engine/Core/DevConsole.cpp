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

	BlinkCursor( deltaSeconds );

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
void DevConsole::ProcessInput()
{
	if ( m_inputSystem == nullptr )
	{
		PrintString( Rgba8::RED, "No input system bound to dev console" );
		return;
	}

	char c;
	while ( m_inputSystem->PopCharacter( &c ) )
	{
		ProcessCharTyped( c );
	}
}


//-----------------------------------------------------------------------------------------------
void DevConsole::PrintString( const Rgba8& textColor, const std::string& devConsolePrintString )
{
	m_logMessages.push_back( DevConsoleLogMessage( textColor, devConsolePrintString ) );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Render( RenderContext& renderer, const Camera& camera, float lineHeight ) const
{
	AABB2 bounds( camera.GetOrthoBottomLeft(), camera.GetOrthoTopRight() );
	Render( renderer, bounds, lineHeight );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Render( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const
{
	if ( !m_isOpen )
	{
		return;
	}

	renderer.BeginCamera( *m_devConsoleCamera );

	AABB2 logMessageBounds = bounds.GetBoxAtTop( .95f );
	AABB2 inputStringBounds = bounds.GetBoxAtBottom( .04f );
	AABB2 inputCarotBounds = bounds.GetBoxAtBottom( .01f );

	//RenderBackground( renderer, bounds );
	RenderLatestLogMessages( renderer, logMessageBounds, lineHeight );
	RenderInputString( renderer, inputStringBounds, lineHeight );
	RenderCursor( renderer, inputCarotBounds, lineHeight );

	renderer.EndCamera( *m_devConsoleCamera );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::BlinkCursor( float deltaSeconds )
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
void DevConsole::RenderBackground( RenderContext& renderer, const AABB2& bounds ) const
{
	Rgba8 backgroundColor = Rgba8::BLACK;
	backgroundColor.a = 150;

	std::vector<Vertex_PCU> backgroundVertices;
	renderer.AppendVertsForAABB2D( backgroundVertices, bounds, backgroundColor );

	//renderer.BindTexture( nullptr );
	renderer.DrawVertexArray( backgroundVertices );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::RenderLatestLogMessages( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const
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

	//BitmapFont* font = renderer.CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	std::vector<Vertex_PCU> vertices;
	float curLineY = 1;
	int latestMessageIndex = (int)m_logMessages.size() - 1;

	for ( int logMessageIndexFromEnd = 0; logMessageIndexFromEnd < numLinesToRender; ++logMessageIndexFromEnd )
	{
		int logMessageIndex = latestMessageIndex - logMessageIndexFromEnd;
		const DevConsoleLogMessage& logMessage = m_logMessages[logMessageIndex];
		
		AppendTextTriangles2D( vertices, logMessage.m_message, Vec2( bounds.mins.x, bounds.mins.y + ( curLineY * lineHeight ) ), lineHeight, logMessage.m_color );
		
		curLineY += lineHeight;
	}
	
	renderer.DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::RenderInputString( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const
{
	std::vector<Vertex_PCU> vertices;

	float cellAspect = .56f;
	float cellWidth = cellAspect * lineHeight;
	float spacingFraction = .2f;
	Vec2 startMins = Vec2( bounds.mins.x, bounds.mins.y );

	AppendTextTriangles2D( vertices, ">", startMins, lineHeight, Rgba8::WHITE, cellAspect, spacingFraction );

	startMins.x += cellWidth + ( cellWidth * spacingFraction );
	AppendTextTriangles2D( vertices, m_currentCommandStr, startMins, lineHeight, Rgba8::WHITE, cellAspect, spacingFraction );

	renderer.DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::RenderCursor( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const
{
	std::vector<Vertex_PCU> vertices;

	float cellAspect = .56f;
	float cellWidth = cellAspect * lineHeight;
	float spacingFraction = .2f;
	Vec2 startMins = Vec2( bounds.mins.x, bounds.mins.y );

	float startCursorPosition = (float)m_currentCursorPosition + 1.f;
	startMins.x += (( startCursorPosition * cellWidth ) + ( startCursorPosition * cellWidth * spacingFraction ) );


	AppendTextTriangles2D( vertices, "_", startMins, lineHeight, m_cursorColor, cellAspect, spacingFraction );

	renderer.DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::ToggleOpenFull()
{
	m_isOpen = !m_isOpen;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::Close()
{
	m_isOpen = false;
}


//-----------------------------------------------------------------------------------------------
void DevConsole::MoveCursorPosition( int deltaCursorPosition )
{
	m_currentCursorPosition += deltaCursorPosition;

	if ( m_currentCursorPosition < 0 )
	{
		m_currentCursorPosition = 0;
	}

	if ( m_currentCursorPosition > (int)m_currentCommandStr.size() )
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

	EventArgs args;
	m_eventSystem->FireEvent( m_currentCommandStr, &args );
}

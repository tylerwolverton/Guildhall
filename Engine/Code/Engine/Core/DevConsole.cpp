#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
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
}


//-----------------------------------------------------------------------------------------------
void DevConsole::BeginFrame()
{

}


//-----------------------------------------------------------------------------------------------
void DevConsole::EndFrame()
{

}


//-----------------------------------------------------------------------------------------------
void DevConsole::Shutdown()
{

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

	RenderBackground( renderer, bounds );
	RenderLatestLogMessages( renderer, bounds, lineHeight );
}


//-----------------------------------------------------------------------------------------------
void DevConsole::RenderBackground( RenderContext& renderer, const AABB2& bounds ) const
{
	Rgba8 backgroundColor = Rgba8::BLACK;
	backgroundColor.a = 150;

	std::vector<Vertex_PCU> backgroundVertexes;
	renderer.AppendVertsForAABB2D( backgroundVertexes, bounds, backgroundColor );

	renderer.BindTexture( nullptr );
	renderer.DrawVertexArray( backgroundVertexes );
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

	BitmapFont* font = renderer.CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	std::vector<Vertex_PCU> vertexes;
	float curLineY = 0;
	int latestMessageIndex = (int)m_logMessages.size() - 1;

	for ( int logMessageIndexFromEnd = 0; logMessageIndexFromEnd < numLinesToRender; ++logMessageIndexFromEnd )
	{
		int logMessageIndex = latestMessageIndex - logMessageIndexFromEnd;
		const DevConsoleLogMessage& logMessage = m_logMessages[logMessageIndex];

		if ( font->GetTexture() != nullptr )
		{
			font->AppendVertsForText2D( vertexes, Vec2( bounds.mins.x, bounds.mins.y + curLineY ), lineHeight, logMessage.m_message, logMessage.m_color, .75f );
		}
		// Use triangle font if no font can be loaded
		else
		{
			AppendTextTriangles2D( vertexes, logMessage.m_message, Vec2( bounds.mins.x, bounds.mins.y + curLineY ), lineHeight, logMessage.m_color );
		}
		curLineY += lineHeight;
	}
	
	if ( font->GetTexture() != nullptr )
	{
		renderer.BindTexture( font->GetTexture() );
	}
	else
	{
		renderer.BindTexture( nullptr );
	}
	renderer.DrawVertexArray( vertexes );
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

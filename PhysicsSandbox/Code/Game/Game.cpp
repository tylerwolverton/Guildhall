#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
Game::Game()
{
} 


//-----------------------------------------------------------------------------------------------
Game::~Game()
{
}


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	m_worldCamera = new Camera();
	m_uiCamera = new Camera();

	m_rng = new RandomNumberGenerator();

	g_devConsole->PrintString( Rgba8::GREEN, "Game Started" );

	RandomizeShapes();

	m_mouseOBB2.SetDimensions( Vec2( .5f, 1.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	delete m_rng;
	m_rng = nullptr;
	
	delete m_uiCamera;
	m_uiCamera = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;
}


//-----------------------------------------------------------------------------------------------
void Game::RestartGame()
{
	Shutdown();
	Startup();
}


//-----------------------------------------------------------------------------------------------
void Game::Update( float deltaSeconds )
{
	UpdateFromKeyboard( deltaSeconds );
	UpdateCameras( deltaSeconds );

	UpdateMousePosition();
	UpdateNearestPoints();
	UpdateShapeColors();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear all screen (backbuffer) pixels to black
	// ALWAYS clear the screen at the top of each frame's Render()!
	g_renderer->ClearScreen( Rgba8::BLACK );

	g_renderer->BeginCamera(*m_worldCamera );
		
	RenderMouseShape();
	RenderShapes();
	RenderNearestPoints();

	g_renderer->EndCamera( *m_worldCamera );

	// Render UI with a new camera
	g_renderer->BeginCamera( *m_uiCamera );

	g_devConsole->Render( *g_renderer, *m_uiCamera, 20 );
	
	g_renderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
}


//-----------------------------------------------------------------------------------------------
void Game::RandomizeShapes()
{
	RandomizeLine();
	RandomizeDisc();
	RandomizeAABB2();
	RandomizeOBB2();
	RandomizeCapsule2();
	RandomizePolygon2();
}


//-----------------------------------------------------------------------------------------------
void Game::RandomizeLine()
{
	m_lineSegmentStart = Vec2( m_rng->RollRandomFloatInRange( 0, WINDOW_WIDTH ),
							   m_rng->RollRandomFloatInRange( 0, WINDOW_HEIGHT ) );

	m_lineSegmentVector = m_rng->RollRandomDirection2D() * m_rng->RollRandomFloatInRange( 1.f, 3.f );
}


//-----------------------------------------------------------------------------------------------
void Game::RandomizeDisc()
{
	m_discCenter = Vec2( m_rng->RollRandomFloatInRange( 0, WINDOW_WIDTH ),
						 m_rng->RollRandomFloatInRange( 0, WINDOW_HEIGHT ) );

	m_discRadius = m_rng->RollRandomFloatInRange( .05f, 1.75f );
}


//-----------------------------------------------------------------------------------------------
void Game::RandomizeAABB2()
{
	Vec2 aabb2Mins( m_rng->RollRandomFloatInRange( 0, WINDOW_WIDTH ),
					m_rng->RollRandomFloatInRange( 0, WINDOW_HEIGHT ) );

	Vec2 minToMaxVector( m_rng->RollRandomFloatInRange( .5f, 3.f ),
						 m_rng->RollRandomFloatInRange( .5f, 3.f ) );

	m_aabb2 = AABB2( aabb2Mins, aabb2Mins + minToMaxVector );
}


//-----------------------------------------------------------------------------------------------
void Game::RandomizeOBB2()
{
	Vec2 aabb2Mins( m_rng->RollRandomFloatInRange( 0, WINDOW_WIDTH ),
					m_rng->RollRandomFloatInRange( 0, WINDOW_HEIGHT ) );

	Vec2 minToMaxVector( m_rng->RollRandomFloatInRange( .5f, 3.f ),
						 m_rng->RollRandomFloatInRange( .5f, 3.f ) );
	
	AABB2 aabb2( aabb2Mins, aabb2Mins + minToMaxVector );

	m_obb2 = OBB2( aabb2, m_rng->RollRandomFloatInRange( 0.f, 360.f ) );
}


//-----------------------------------------------------------------------------------------------
void Game::RandomizeCapsule2()
{
	Vec2 capsuleMiddleStart( m_rng->RollRandomFloatInRange( 0, WINDOW_WIDTH ),
							 m_rng->RollRandomFloatInRange( 0, WINDOW_HEIGHT ) );

	Vec2 capsuleMiddleEnd = capsuleMiddleStart + ( m_rng->RollRandomDirection2D() * m_rng->RollRandomFloatInRange( 1.f, 3.f ) );

	float radius = m_rng->RollRandomFloatInRange( .25f, 1.f );

	m_capsule2 = Capsule2( capsuleMiddleStart, capsuleMiddleEnd, radius );
}


//-----------------------------------------------------------------------------------------------
void Game::RandomizePolygon2()
{
	Vec2 polygonStart( m_rng->RollRandomFloatInRange( 0, WINDOW_WIDTH ),
					   m_rng->RollRandomFloatInRange( 0, WINDOW_HEIGHT ) );

	// Since there is no CCW verification, just hard code points and randomize starting pos
	std::vector<Vec2> points{
		polygonStart + Vec2( 0.f, 0.f ),
		polygonStart + Vec2( 2.f, 1.f ),
		polygonStart + Vec2( 4.f, 3.f ),
		polygonStart + Vec2( 2.5f, 4.5f ),
		polygonStart + Vec2( 1.5f, 1.f )
	};

	m_polygon2.SetPoints( points );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderMouseShape() const
{
	switch ( m_mouseState )
	{
		case MOUSE_STATE_POINT:
			{
				g_renderer->BindTexture( nullptr );
				g_renderer->DrawRing2D( m_mouseWorldPosition, .02f, Rgba8::WHITE, .04f );
			}
			break;

		case MOUSE_STATE_OBB2:
			{
				g_renderer->BindTexture( nullptr );
				g_renderer->DrawOBB2( m_mouseOBB2, Rgba8::WHITE );
			}
			break;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::RenderShapes() const
{
	g_renderer->BindTexture( nullptr );

	RenderLine();
	RenderDisc();
	RenderAABB2();
	RenderOBB2();
	RenderCapsule2();
	RenderPolygon2();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderLine() const
{
	g_renderer->DrawLine2D( m_lineSegmentStart, m_lineSegmentStart + m_lineSegmentVector, m_lineColor, .01f );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderDisc() const
{
	g_renderer->DrawDisc2D( m_discCenter, m_discRadius, m_discColor );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderAABB2() const
{
	g_renderer->DrawAABB2( m_aabb2, m_AABB2Color );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderOBB2() const
{
	g_renderer->DrawOBB2( m_obb2, m_OBB2Color );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderCapsule2() const
{
	g_renderer->DrawCapsule2D( m_capsule2, m_capsule2Color );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderPolygon2() const
{
	g_renderer->DrawPolygon2( m_polygon2, m_polygon2Color );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderNearestPoints() const 
{
	if ( m_mouseState == MOUSE_STATE_POINT )
	{
		// Line
		g_renderer->DrawRing2D( m_nearestPointOnLine, .02f, Rgba8::WHITE, .04f );
		g_renderer->DrawLine2D( m_mouseWorldPosition, m_nearestPointOnLine, Rgba8::YELLOW, .025f );

		// Disc
		g_renderer->DrawRing2D( m_nearestPointOnDisc, .02f, Rgba8::WHITE, .04f );
		g_renderer->DrawLine2D( m_mouseWorldPosition, m_nearestPointOnDisc, Rgba8::YELLOW, .025f );

		// AABB2
		g_renderer->DrawRing2D( m_nearestPointOnAABB2, .02f, Rgba8::WHITE, .04f );
		g_renderer->DrawLine2D( m_mouseWorldPosition, m_nearestPointOnAABB2, Rgba8::YELLOW, .025f );

		// OBB2
		g_renderer->DrawRing2D( m_nearestPointOnOBB2, .02f, Rgba8::WHITE, .04f );
		g_renderer->DrawLine2D( m_mouseWorldPosition, m_nearestPointOnOBB2, Rgba8::YELLOW, .025f );

		// Capsule2
		g_renderer->DrawRing2D( m_nearestPointOnCapsule2, .02f, Rgba8::WHITE, .04f );
		g_renderer->DrawLine2D( m_mouseWorldPosition, m_nearestPointOnCapsule2, Rgba8::YELLOW, .025f );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
	{
		m_isDebugRendering = !m_isDebugRendering;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F2 )
		 || g_inputSystem->WasKeyJustPressed( MOUSE_MBUTTON ) )
	{
		switch ( m_mouseState )
		{
			case MOUSE_STATE_POINT:
				m_mouseState = MOUSE_STATE_OBB2;
				break;

			case MOUSE_STATE_OBB2:
				m_mouseState = MOUSE_STATE_POINT;
				break;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F5 ) )
	{
		RandomizeShapes();
	}

	if ( g_inputSystem->IsKeyPressed( 'Q' )
		 || g_inputSystem->IsKeyPressed( MOUSE_LBUTTON ) )
	{
		m_mouseOBB2.SetOrientationDegrees( m_mouseOBB2.GetOrientationDegrees() + ( 50.f * deltaSeconds ) );
	}

	if ( g_inputSystem->IsKeyPressed( 'E' )
		 || g_inputSystem->IsKeyPressed( MOUSE_RBUTTON ) )
	{
		m_mouseOBB2.SetOrientationDegrees( m_mouseOBB2.GetOrientationDegrees() - ( 50.f * deltaSeconds ) );
	}

	float mouseWheelScrollAmount = g_inputSystem->GetMouseWheelScrollAmountDelta();
	if ( mouseWheelScrollAmount > .001f
		 || mouseWheelScrollAmount < -.001f )
	{
		m_mouseOBB2.RotateByDegrees( g_inputSystem->GetMouseWheelScrollAmountDelta() * 10.f );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	m_worldCamera->SetOrthoView( Vec2::ZERO, Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );
	m_uiCamera->SetOrthoView( Vec2( 0.f, 0.f ), Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMousePosition()
{
	m_mouseWorldPosition = g_inputSystem->GetNormalizedMouseClientPos();
	m_mouseWorldPosition.x *= WINDOW_WIDTH;
	m_mouseWorldPosition.y *= WINDOW_HEIGHT;

	m_mouseOBB2.SetCenter( m_mouseWorldPosition );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateNearestPoints()
{
	m_nearestPointOnLine = GetNearestPointOnLineSegment2D( m_mouseWorldPosition, m_lineSegmentStart, m_lineSegmentStart + m_lineSegmentVector );
	m_nearestPointOnDisc = GetNearestPointOnDisc2D( m_mouseWorldPosition, m_discCenter, m_discRadius );
	m_nearestPointOnAABB2 = GetNearestPointOnAABB2D( m_mouseWorldPosition, m_aabb2 );
	m_nearestPointOnOBB2 = GetNearestPointOnOBB2D( m_mouseWorldPosition, m_obb2 );
	m_nearestPointOnCapsule2 = GetNearestPointOnCapsule2D( m_mouseWorldPosition, m_capsule2 );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateShapeColors()
{
	switch ( m_mouseState )
	{
		case MOUSE_STATE_POINT:
		{
			m_lineColor.a = 175;
			m_discColor.a = IsPointInsideDisc( m_mouseWorldPosition, m_discCenter, m_discRadius ) ? 255 : 175;
			m_AABB2Color.a = IsPointInsideAABB2D( m_mouseWorldPosition, m_aabb2 ) ? 255 : 175;
			m_OBB2Color.a = IsPointInsideOBB2D( m_mouseWorldPosition, m_obb2 ) ? 255 : 175;
			m_capsule2Color.a = IsPointInsideCapsule2D( m_mouseWorldPosition, m_capsule2 ) ? 255 : 175;
		}
		break;

		case MOUSE_STATE_OBB2:
		{
			m_lineColor.a = DoOBBAndLineSegmentOverlap2D(m_mouseOBB2, m_lineSegmentStart, m_lineSegmentStart + m_lineSegmentVector) ? 255 : 175;
			m_discColor.a = DoOBBAndDiscOverlap2D( m_mouseOBB2, m_discCenter, m_discRadius ) ? 255 : 175;
			m_AABB2Color.a = DoOBBAndAABBOverlap2D( m_mouseOBB2, m_aabb2 ) ? 255 : 175;
			m_OBB2Color.a = DoOBBAndOBBOverlap2D( m_mouseOBB2, m_obb2 ) ? 255 : 175;
			m_capsule2Color.a = DoOBBAndCapsuleOverlap2D( m_mouseOBB2, m_capsule2 ) ? 255 : 175;
		}
		break;
	}
}

#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"

#include "Game/GameCommon.hpp"
#include "Game/GameObject.hpp"


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
	m_worldCamera->SetOutputSize( Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );
	m_worldCamera->SetPosition( m_focalPoint );
	
	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) );
	m_uiCamera->SetPosition( Vec3( WINDOW_WIDTH_PIXELS * .5f, WINDOW_HEIGHT_PIXELS * .5f, 0.f ) );

	m_rng = new RandomNumberGenerator();

	m_physics2D = new Physics2D();

	m_mouseHistoryPoints[0].position = Vec2::ZERO;
	m_mouseHistoryPoints[0].deltaSeconds = 0.f;
	m_mouseHistoryPoints[1].position = Vec2::ZERO;
	m_mouseHistoryPoints[1].deltaSeconds = 0.f;
	m_mouseHistoryPoints[2].position = Vec2::ZERO;
	m_mouseHistoryPoints[2].deltaSeconds = 0.f;
	m_mouseHistoryPoints[3].position = Vec2::ZERO;
	m_mouseHistoryPoints[3].deltaSeconds = 0.f;
	m_mouseHistoryPoints[4].position = Vec2::ZERO;
	m_mouseHistoryPoints[4].deltaSeconds = 0.f;

	g_devConsole->PrintString( Rgba8::GREEN, "Game Started" );
}


//-----------------------------------------------------------------------------------------------
void Game::BeginFrame()
{
	m_physics2D->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	delete m_physics2D;
	m_physics2D = nullptr;

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
	UpdateMouse( deltaSeconds );

	UpdateCameras( deltaSeconds );

	UpdateGameObjects();
	UpdateDraggedObject();
	UpdatePotentialPolygon();
	UpdateOffScreenGameObjects();

	m_physics2D->Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Clear all screen (backbuffer) pixels to black
	// ALWAYS clear the screen at the top of each frame's Render()!
	g_renderer->ClearScreen( Rgba8::BLACK );

	g_renderer->BeginCamera( *m_worldCamera );
		
	RenderGameObjects();

	if ( m_gameState == eGameState::CREATE_POLYGON )
	{
		RenderPolygonPoints();
	}

	g_renderer->EndCamera( *m_worldCamera );

	g_renderer->BeginCamera( *m_uiCamera );
	
	RenderUI();
	
	g_renderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
}


//-----------------------------------------------------------------------------------------------
void Game::EndFrame()
{
	PerformGarbageCollection();

	m_physics2D->EndFrame();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderGameObjects() const
{
	for ( int objectIdx = 0; objectIdx < (int)m_gameObjects.size(); ++objectIdx )
	{
		GameObject* gameObject = m_gameObjects[ objectIdx ];
		if ( gameObject == nullptr )
		{
			continue;
		}

		gameObject->DebugRender();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::RenderPolygonPoints() const
{
	int numPotentialPoints = (int)m_potentialPolygonPoints.size();
	if ( numPotentialPoints < 1 )
	{
		return;
	}

	for ( int pointIdx = 0; pointIdx < numPotentialPoints - 1; ++pointIdx )
	{
		g_renderer->DrawDisc2D( m_potentialPolygonPoints[pointIdx], .05f, Rgba8::WHITE );
		
		if ( pointIdx > 0 )
		{
			int lastIdx = pointIdx - 1;
			g_renderer->DrawLine2D( m_potentialPolygonPoints[lastIdx], m_potentialPolygonPoints[pointIdx], Rgba8::BLUE, .05f );
		}
	}

	Rgba8 lineColor = ( m_isPotentialPolygonConvex || numPotentialPoints < 3 ) ? Rgba8::BLUE : Rgba8::RED;

	int lastPointIdx = numPotentialPoints - 2;
	g_renderer->DrawLine2D( m_potentialPolygonPoints[lastPointIdx], m_mouseWorldPosition, lineColor, .05f );
}


//-----------------------------------------------------------------------------------------------
void Game::RenderUI() const
{
	std::string gravityStr = Stringf( "Gravity: %.1f", m_physics2D->GetSceneGravity().y );
	DrawTextTriangles2D( *g_renderer, gravityStr, Vec2( 1600.f, 1000.f ), 30.f, Rgba8::WHITE );
	DrawTextTriangles2D( *g_renderer, "- or + keys to change ", Vec2( 1600.f, 970.f ), 20.f, Rgba8::WHITE );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	switch ( m_gameState )
	{
		case eGameState::SANDBOX:
		{
			if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
			{
				m_isDebugRendering = !m_isDebugRendering;
			}

			if ( g_inputSystem->IsKeyPressed( 'W' ) )
			{
				m_focalPoint.y += 5.f * deltaSeconds;
			}
			if ( g_inputSystem->IsKeyPressed( 'A' ) )
			{
				m_focalPoint.x -= 5.f * deltaSeconds;
			}
			if ( g_inputSystem->IsKeyPressed( 'S' ) )
			{
				m_focalPoint.y -= 5.f * deltaSeconds;
			}
			if ( g_inputSystem->IsKeyPressed( 'D' ) )
			{
				m_focalPoint.x += 5.f * deltaSeconds;
			}

			if ( g_inputSystem->WasKeyJustPressed( '1' ) )
			{
				if ( m_isMouseDragging )
				{
					if ( m_dragTarget != nullptr )
					{
						m_dragTarget->SetSimulationMode( SIMULATION_MODE_STATIC );
					}
				}
				else
				{
					float radius = m_rng->RollRandomFloatInRange( .25f, 1.f );
					SpawnDisc( m_mouseWorldPosition, radius );
				}
			}

			if ( g_inputSystem->WasKeyJustPressed( '2' ) )
			{
				if ( m_isMouseDragging )
				{
					if ( m_dragTarget != nullptr )
					{
						m_dragTarget->SetSimulationMode( SIMULATION_MODE_KINEMATIC );
					}
				}
				else
				{
					m_gameState = eGameState::CREATE_POLYGON;
					m_potentialPolygonPoints.push_back( m_mouseWorldPosition );
					m_potentialPolygonPoints.push_back( m_mouseWorldPosition );
				}
			}

			if ( g_inputSystem->WasKeyJustPressed( '3' ) )
			{
				if ( m_isMouseDragging )
				{
					if ( m_dragTarget != nullptr )
					{
						m_dragTarget->SetSimulationMode( SIMULATION_MODE_DYNAMIC );
					}
				}
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_PLUS ) )
			{
				Vec2 currentGravity = m_physics2D->GetSceneGravity();
				m_physics2D->SetSceneGravity( currentGravity + Vec2( 0.f, -1.f ) );
			}
			
			if ( g_inputSystem->WasKeyJustPressed( KEY_MINUS ) )
			{
				Vec2 currentGravity = m_physics2D->GetSceneGravity();
				m_physics2D->SetSceneGravity( currentGravity + Vec2( 0.f, 1.f ) );
			}

			if ( g_inputSystem->WasKeyJustPressed( 'O' ) )
			{
				m_focalPoint = Vec3::ZERO;
				m_zoomFactor = 1.f;
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_BACKSPACE )
				 || g_inputSystem->WasKeyJustPressed( KEY_DELETE ) )
			{
				if ( m_dragTarget != nullptr )
				{
					int index = GetIndexOfGameObject( m_dragTarget );

					GUARANTEE_OR_DIE( index != -1, "Dragged object isn't in game object list" );

					m_garbageGameObjectIndexes.push_back( index );

					m_isMouseDragging = false;
					m_dragTarget = nullptr;
				}
			}
		} break;

		case eGameState::CREATE_POLYGON:
		{
			if ( g_inputSystem->ConsumeWasKeyJustPressed( KEY_ESC ) )
			{
				m_potentialPolygonPoints.clear();
				m_gameState = eGameState::SANDBOX;
			}
		} break;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	m_worldCamera->SetPosition( m_focalPoint );
	m_worldCamera->SetProjectionOrthographic( WINDOW_HEIGHT * m_zoomFactor );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouse( float deltaSeconds )
{
	UpdateMouseHistory( m_mouseWorldPosition, deltaSeconds );
	
	m_mouseWorldPosition = g_inputSystem->GetNormalizedMouseClientPos();
	m_mouseWorldPosition = m_worldCamera->ClientToWorldPosition( m_mouseWorldPosition );

	switch ( m_gameState )
	{
		case eGameState::SANDBOX:
		{
			if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
			{
				m_isMouseDragging = true;
				m_dragTarget = GetTopGameObjectAtMousePosition();
				if ( m_dragTarget != nullptr )
				{
					m_dragOffset = m_mouseWorldPosition - m_dragTarget->GetPosition();
					m_dragTarget->DisablePhysics();
				}
			}
			else if ( g_inputSystem->WasKeyJustReleased( MOUSE_LBUTTON ) )
			{
				if ( m_isMouseDragging )
				{
					if ( m_dragTarget != nullptr )
					{
						MouseMovementHistoryPoint historyPt = GetCummulativeMouseHistory();

						m_dragTarget->SetVelocity( historyPt.position / historyPt.deltaSeconds );
						m_dragTarget->EnablePhysics();
					}

					m_isMouseDragging = false;
					m_dragTarget = nullptr;
				}
			}
		} break;

		case eGameState::CREATE_POLYGON:
		{
			if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
			{
				if ( (int)m_potentialPolygonPoints.size() < 3
					 || m_isPotentialPolygonConvex )
				{
					m_potentialPolygonPoints.push_back( m_mouseWorldPosition );
				}
			}

			if ( g_inputSystem->WasKeyJustPressed( MOUSE_RBUTTON ) )
			{
				m_potentialPolygonPoints.pop_back();

				SpawnPolygon( m_potentialPolygonPoints );

				m_potentialPolygonPoints.clear();
				m_gameState = eGameState::SANDBOX;
			}
		} break;
	}

	float mouseWheelScrollAmount = g_inputSystem->GetMouseWheelScrollAmountDelta();
	if ( mouseWheelScrollAmount > .001f
		 || mouseWheelScrollAmount < -.001f )
	{
		m_zoomFactor -= mouseWheelScrollAmount * .25f;
		m_zoomFactor = ClampMinMax( m_zoomFactor, .1f, 20.f );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseHistory( const Vec2& position, float deltaSeconds )
{
	for ( int historyIdx = 0; historyIdx < 4; ++historyIdx )
	{
		int nextHistoryPoint = historyIdx + 1;
		m_mouseHistoryPoints[historyIdx] = m_mouseHistoryPoints[nextHistoryPoint];
	}

	m_mouseHistoryPoints[4].position = position;
	m_mouseHistoryPoints[4].deltaSeconds = deltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateGameObjects()
{
	ResetGameObjectColors();
	CheckCollisions();	
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateDraggedObject()
{
	if ( m_dragTarget != nullptr )
	{
		m_dragTarget->SetPosition( m_mouseWorldPosition - m_dragOffset );

		m_dragTarget->SetBorderColor( Rgba8::DARK_GREEN );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdatePotentialPolygon()
{
	if ( m_gameState == eGameState::CREATE_POLYGON )
	{
		if ( m_potentialPolygonPoints.size() > 0
			 && m_potentialPolygonPoints.size() < 3 )
		{
			m_isPotentialPolygonConvex = true;
		}
		else
		{
			Polygon2 potentialPolygon( m_potentialPolygonPoints );
			m_isPotentialPolygonConvex = potentialPolygon.IsConvex();
		}

		int lastPointIdx = (int)m_potentialPolygonPoints.size() - 1;
		m_potentialPolygonPoints[lastPointIdx] = m_mouseWorldPosition;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateOffScreenGameObjects()
{
	AABB2 screenBounds( m_worldCamera->GetOrthoMin(), m_worldCamera->GetOrthoMax() );

	for ( int objectIdx = 0; objectIdx < (int)m_gameObjects.size(); ++objectIdx )
	{
		if ( m_gameObjects[objectIdx] == nullptr )
		{
			continue;
		}

		GameObject*& gameObject = m_gameObjects[objectIdx];

		const AABB2 objectBoundingBox = gameObject->GetBoundingBox();

		// Check bouncing off bottom
		unsigned int edgesOfScreenColliderIsOff = gameObject->CheckIfOutsideScreen( screenBounds, false );
		if ( edgesOfScreenColliderIsOff & SCREEN_EDGE_BOTTOM
			 && gameObject->GetVelocity().y < 0.01f)
		{
			Vec2 newVelocity( gameObject->GetVelocity() );
			newVelocity.y = abs( newVelocity.y );

			gameObject->SetVelocity( newVelocity );
		}
		
		// Check side wraparound
		edgesOfScreenColliderIsOff = gameObject->CheckIfOutsideScreen( screenBounds, true );
		float yPos = gameObject->GetPosition().y;

		if ( edgesOfScreenColliderIsOff & SCREEN_EDGE_LEFT )
		{
			float xDistanceToOtherSideOfScreen = objectBoundingBox.mins.x - screenBounds.maxs.x;
			gameObject->SetPosition( Vec2( gameObject->GetPosition().x - xDistanceToOtherSideOfScreen, yPos ) );
		}
		if( edgesOfScreenColliderIsOff & SCREEN_EDGE_RIGHT )
		{
			float xDistanceToOtherSideOfScreen = objectBoundingBox.maxs.x - screenBounds.mins.x;
			gameObject->SetPosition( Vec2( gameObject->GetPosition().x - xDistanceToOtherSideOfScreen, yPos ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::ResetGameObjectColors()
{
	// Reset fill color for all game objects
	for ( int objectIdx = (int)m_gameObjects.size() - 1; objectIdx >= 0; --objectIdx )
	{
		GameObject* gameObject = m_gameObjects[objectIdx];
		if ( gameObject == nullptr )
		{
			continue;
		}

		// Default fill color to white
		gameObject->SetFillColor( Rgba8::WHITE );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::CheckCollisions()
{
	for ( int objectIdx = (int)m_gameObjects.size() - 1; objectIdx >= 0; --objectIdx )
	{
		GameObject* gameObject = m_gameObjects[objectIdx];
		if ( gameObject == nullptr )
		{
			continue;
		}

		UpdateBorderColor( gameObject );

		// Check intersection with other game objects
		for ( int otherObjIdx = (int)m_gameObjects.size() - 1; otherObjIdx >= 0; --otherObjIdx )
		{
			GameObject* otherGameObject = m_gameObjects[otherObjIdx];
			HandleIntersection( gameObject, otherGameObject );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateBorderColor( GameObject* gameObject )
{
	if ( gameObject == nullptr )
	{
		return;
	}
	
	if ( gameObject->Contains( m_mouseWorldPosition ) )
	{
		gameObject->SetBorderColor( Rgba8::YELLOW );
	}
	else
	{
		gameObject->SetBorderColor( Rgba8::BLUE );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::HandleIntersection( GameObject* gameObject, GameObject* otherGameObject )
{
	if ( otherGameObject == nullptr
		 || gameObject == otherGameObject )
	{
		return;
	}

	// Highlight red if intersecting another object
	if ( gameObject->Intersects( otherGameObject ) )
	{
		gameObject->SetFillColor( Rgba8::RED );
		otherGameObject->SetFillColor( Rgba8::RED );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnDisc( const Vec2& center, float radius )
{
	GameObject* gameObject = new GameObject();
	gameObject->SetRigidbody( m_physics2D->CreateRigidbody() );
	gameObject->SetPosition( center );

	DiscCollider2D* discCollider = m_physics2D->CreateDiscCollider( Vec2::ZERO, radius );
	gameObject->SetCollider( discCollider );

	gameObject->SetBorderColor( Rgba8::BLUE );
	gameObject->SetFillColor( Rgba8::WHITE );

	m_gameObjects.push_back( gameObject );
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnPolygon( const Polygon2& polygon )
{
	GameObject* gameObject = new GameObject();
	gameObject->SetRigidbody( m_physics2D->CreateRigidbody() );
	gameObject->SetPosition( polygon.GetCenterOfMass() );

	PolygonCollider2D* polygonCollider = m_physics2D->CreatePolygon2Collider( polygon );
	gameObject->SetCollider( polygonCollider );

	gameObject->SetBorderColor( Rgba8::BLUE );
	gameObject->SetFillColor( Rgba8::WHITE );

	m_gameObjects.push_back( gameObject );
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnPolygon( const std::vector<Vec2>& points )
{
	Polygon2 newPolygon2 = Polygon2( points );

	if ( !newPolygon2.IsConvex() )
	{
		g_devConsole->PrintString( Rgba8::YELLOW, "Invalid polygon cannot be spawned" );
		return;
	}

	SpawnPolygon( newPolygon2 );
}


//-----------------------------------------------------------------------------------------------
GameObject* Game::GetTopGameObjectAtMousePosition()
{
	int gameObjectIdx = GetIndexOfTopGameObjectAtMousePosition();

	if ( gameObjectIdx == -1 )
	{
		return nullptr;
	}
	else
	{
		return m_gameObjects[ gameObjectIdx ];
	}
}


//-----------------------------------------------------------------------------------------------
int Game::GetIndexOfTopGameObjectAtMousePosition()
{
	for ( int objectIdx = (int)m_gameObjects.size() - 1; objectIdx >= 0; --objectIdx )
	{
		GameObject* gameObject = m_gameObjects[ objectIdx ];
		if ( gameObject == nullptr )
		{
			continue;
		}

		if ( gameObject->Contains( m_mouseWorldPosition ) )
		{
			return objectIdx;
		}
	}

	return -1;
}


//-----------------------------------------------------------------------------------------------
int Game::GetIndexOfGameObject( GameObject* gameObjectToFind )
{
	if ( gameObjectToFind == nullptr )
	{
		return -1;
	}

	for ( int objectIdx = (int)m_gameObjects.size() - 1; objectIdx >= 0; --objectIdx )
	{
		GameObject* gameObject = m_gameObjects[ objectIdx ];
		if ( gameObject == nullptr )
		{
			continue;
		}

		if ( gameObject == gameObjectToFind )
		{
			return objectIdx;
		}
	}

	return -1;
}


//-----------------------------------------------------------------------------------------------
void Game::PerformGarbageCollection()
{
	for ( int garbageIdx = 0; garbageIdx < (int)m_garbageGameObjectIndexes.size(); ++garbageIdx )
	{
		GameObject*& garbageGameObject = m_gameObjects[m_garbageGameObjectIndexes[garbageIdx]];

		if ( garbageGameObject == m_dragTarget )
		{
			m_dragTarget = nullptr;
		}

		delete garbageGameObject;
		garbageGameObject = nullptr;
	}

	m_garbageGameObjectIndexes.clear();
}


//-----------------------------------------------------------------------------------------------
MouseMovementHistoryPoint Game::GetCummulativeMouseHistory()
{
	MouseMovementHistoryPoint history;
	history.position = m_mouseHistoryPoints[4].position - m_mouseHistoryPoints[0].position;
	history.deltaSeconds = 0.f;

	for ( int historyIdx = 0; historyIdx < 5; ++historyIdx )
	{
		history.deltaSeconds += m_mouseHistoryPoints[historyIdx].deltaSeconds;
	}

	return history;
}

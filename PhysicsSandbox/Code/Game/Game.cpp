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
	
	m_rng = new RandomNumberGenerator();

	m_physics2D = new Physics2D();

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
	UpdateMouse();

	UpdateCameras( deltaSeconds );

	UpdateGameObjects();
	UpdateDraggedObject();
	UpdatePotentialPolygon();

	m_physics2D->Update( deltaSeconds );

	UpdateBouncingGameObjects();
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

	for ( int pointIdx = 0; pointIdx < numPotentialPoints; ++pointIdx )
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
				float radius = m_rng->RollRandomFloatInRange( .25f, 1.f );
				SpawnDisc( m_mouseWorldPosition, radius );
			}

			if ( g_inputSystem->WasKeyJustPressed( '2' ) )
			{
				m_gameState = eGameState::CREATE_POLYGON;
				m_potentialPolygonPoints.push_back( m_mouseWorldPosition );
				m_potentialPolygonPoints.push_back( m_mouseWorldPosition );
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
void Game::UpdateMouse()
{
	m_lastMouseWorldPosition = m_mouseWorldPosition;
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
					m_dragOffset = m_mouseWorldPosition - m_dragTarget->m_rigidbody->GetPosition();
				}
			}
			else if ( g_inputSystem->WasKeyJustReleased( MOUSE_LBUTTON ) )
			{
				if ( m_isMouseDragging )
				{
					if ( m_dragTarget != nullptr )
					{
						m_dragTarget->m_rigidbody->SetVelocity( ( m_mouseWorldPosition - m_lastMouseWorldPosition ) * 5.f);
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
				if ( m_isPotentialPolygonConvex )
				{
					m_potentialPolygonPoints.push_back( m_mouseWorldPosition );

					SpawnPolygon( m_potentialPolygonPoints );

					m_potentialPolygonPoints.clear();
					m_gameState = eGameState::SANDBOX;
				}
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
void Game::UpdateGameObjects()
{
	for ( int objectIdx = (int)m_gameObjects.size() - 1; objectIdx >= 0; --objectIdx )
	{
		GameObject* gameObject = m_gameObjects[ objectIdx ];
		if ( gameObject == nullptr )
		{
			continue;
		}

		// Default fill color to white
		gameObject->m_fillColor = Rgba8::WHITE;

		// Change border color based on mouse position
		DiscCollider2D* collider = (DiscCollider2D*)gameObject->m_rigidbody->m_collider;
		if ( collider->Contains( m_mouseWorldPosition ) )
		{
			gameObject->m_borderColor = Rgba8::YELLOW;
		}
		else
		{
			gameObject->m_borderColor = Rgba8::BLUE;
		}

		// Check intersection with other game objects
		for ( int otherObjIdx = (int)m_gameObjects.size() - 1; otherObjIdx >= 0; --otherObjIdx )
		{
			GameObject* otherGameObject = m_gameObjects[ otherObjIdx ];
			if ( otherGameObject == nullptr
				 || gameObject == otherGameObject )
			{
				continue;
			}

			// Highlight red if intersecting another object
			if ( collider->Intersects( (DiscCollider2D*)otherGameObject->m_rigidbody->m_collider ) )
			{
				gameObject->m_fillColor = Rgba8::RED;
				otherGameObject->m_fillColor = Rgba8::RED;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateDraggedObject()
{
	if ( m_dragTarget != nullptr )
	{
		m_dragTarget->m_rigidbody->SetPosition( m_mouseWorldPosition - m_dragOffset );

		m_dragTarget->m_borderColor = Rgba8::DARK_GREEN;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdatePotentialPolygon()
{
	if ( m_gameState == eGameState::CREATE_POLYGON
		 && m_potentialPolygonPoints.size() > 0 )
	{
		//std::vector<Vec2> potentialPotentialPoints( m_potentialPolygonPoints );

		Polygon2 potentialPolygon( m_potentialPolygonPoints );
		m_isPotentialPolygonConvex = potentialPolygon.IsConvex();

		int lastPointIdx = m_potentialPolygonPoints.size() - 1;
		m_potentialPolygonPoints[lastPointIdx] = m_mouseWorldPosition;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateBouncingGameObjects()
{
	for ( int objectIdx = 0; objectIdx < (int)m_gameObjects.size(); ++objectIdx )
	{
		if ( m_gameObjects[objectIdx] == nullptr )
		{
			continue;
		}

		if ( m_gameObjects[objectIdx]->m_rigidbody->GetPosition().y < m_worldCamera->GetOrthoMin().y )
		{
			Vec2 newVelocity( m_gameObjects[objectIdx]->m_rigidbody->GetVelocity() );
			newVelocity.y *= -1.f;

			m_gameObjects[objectIdx]->m_rigidbody->SetVelocity( newVelocity );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnDisc( const Vec2& center, float radius )
{
	GameObject* gameObject = new GameObject();
	gameObject->m_rigidbody = m_physics2D->CreateRigidbody();
	gameObject->m_rigidbody->SetPosition( center );

	DiscCollider2D* discCollider = m_physics2D->CreateDiscCollider( Vec2::ZERO, radius );
	gameObject->m_rigidbody->TakeCollider( discCollider );

	gameObject->m_borderColor = Rgba8::BLUE;
	gameObject->m_fillColor = Rgba8::WHITE;

	m_gameObjects.push_back( gameObject );
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnPolygon( const Polygon2& polygon )
{
	GameObject* gameObject = new GameObject();
	gameObject->m_rigidbody = m_physics2D->CreateRigidbody();
	// TODO: Make the center of mass the rigidbody location
	gameObject->m_rigidbody->SetPosition( polygon.GetCenterOfMass() );

	PolygonCollider2D* polygonCollider = m_physics2D->CreatePolygon2Collider( polygon );
	gameObject->m_rigidbody->TakeCollider( polygonCollider );

	gameObject->m_borderColor = Rgba8::BLUE;
	gameObject->m_fillColor = Rgba8::WHITE;

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

		DiscCollider2D* collider = (DiscCollider2D*)gameObject->m_rigidbody->m_collider;

		if ( collider->Contains( m_mouseWorldPosition ) )
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

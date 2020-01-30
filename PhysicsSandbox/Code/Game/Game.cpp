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
	m_uiCamera = new Camera();

	m_rng = new RandomNumberGenerator();

	m_physics2D = new Physics2D();

	g_devConsole->PrintString( Rgba8::GREEN, "Game Started" );
	
	m_mouseOBB2.SetDimensions( Vec2( .5f, 1.f ) );
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
	UpdateCameras( deltaSeconds );

	UpdateMouse();
	UpdateGameObjects();
	UpdateDraggedObject();

	m_physics2D->Update();
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
void Game::EndFrame()
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

	m_physics2D->EndFrame();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderMouseShape() const
{
	/*switch ( m_mouseState )
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
	}*/
}


//-----------------------------------------------------------------------------------------------
void Game::RenderShapes() const
{
	for ( int shapeIdx = 0; shapeIdx < (int)m_gameObjects.size(); ++shapeIdx )
	{
		GameObject* gameObject = m_gameObjects[shapeIdx];
		if ( gameObject == nullptr )
		{
			continue;
		}

		DiscCollider2D* collider = (DiscCollider2D*)gameObject->m_rigidbody->m_collider;
		Rgba8 fillColor = gameObject->m_fillColor;
		fillColor.a *= .5f;
		collider->DebugRender( g_renderer, gameObject->m_borderColor, fillColor );
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

	if ( g_inputSystem->WasKeyJustPressed( '1' ) )
	{
		float radius = m_rng->RollRandomFloatInRange( .25f, 1.f );
		SpawnDisc( m_mouseWorldPosition, radius );
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_BACKSPACE )
		 || g_inputSystem->WasKeyJustPressed( KEY_DELETE ))
	{
		// TODO: Only delete if dragging?
		int objectToDeleteIndex = GetIndexOfTopGameObjectAtMousePosition();

		if ( objectToDeleteIndex != -1 )
		{
			m_garbageGameObjectIndexes.push_back( objectToDeleteIndex );
		}
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
void Game::UpdateMouse()
{
	m_mouseWorldPosition = g_inputSystem->GetNormalizedMouseClientPos();
	m_mouseWorldPosition.x *= WINDOW_WIDTH;
	m_mouseWorldPosition.y *= WINDOW_HEIGHT;

	/*m_mouseOBB2.SetCenter( m_mouseWorldPosition );*/

	if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
	{
		m_isMouseDragging = true;
		m_dragTarget = GetTopGameObjectAtMousePosition();
		m_dragOffset = m_mouseWorldPosition - m_dragTarget->m_rigidbody->m_worldPosition;
	}
	else if ( g_inputSystem->WasKeyJustReleased( MOUSE_LBUTTON ) )
	{
		m_isMouseDragging = false;
		m_dragTarget = nullptr;
	}

	if ( g_inputSystem->WasKeyJustPressed( MOUSE_RBUTTON ) )
	{
		
	}

	float mouseWheelScrollAmount = g_inputSystem->GetMouseWheelScrollAmountDelta();
	if ( mouseWheelScrollAmount > .001f
		 || mouseWheelScrollAmount < -.001f )
	{
		
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
		m_dragTarget->m_rigidbody->m_worldPosition = m_mouseWorldPosition - m_dragOffset;
		m_dragTarget->m_rigidbody->m_collider->UpdateWorldShape();

		m_dragTarget->m_borderColor = Rgba8::GREEN;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SpawnDisc( const Vec2& center, float radius )
{
	GameObject* gameObject = new GameObject();
	gameObject->m_rigidbody = m_physics2D->CreateRigidbody();
	gameObject->m_rigidbody->m_worldPosition = center;

	DiscCollider2D* discCollider = m_physics2D->CreateDiscCollider( Vec2::ZERO, radius );
	gameObject->m_rigidbody->TakeCollider( discCollider );

	gameObject->m_borderColor = Rgba8::BLUE;
	gameObject->m_fillColor = Rgba8::WHITE;

	m_gameObjects.push_back( gameObject );
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
	for ( int shapeIdx = (int)m_gameObjects.size() - 1; shapeIdx >= 0; --shapeIdx )
	{
		GameObject* gameObject = m_gameObjects[shapeIdx];
		if ( gameObject == nullptr )
		{
			continue;
		}

		DiscCollider2D* collider = (DiscCollider2D*)gameObject->m_rigidbody->m_collider;

		if ( collider->Contains( m_mouseWorldPosition ) )
		{
			return shapeIdx;
		}
	}

	return -1;
}

#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"

#include "Game/Map.hpp"

//-----------------------------------------------------------------------------------------------
Entity::Entity( const EntityDefinition& entityDef, Map* map )
	: ZephyrEntity( entityDef )
	, m_entityDef( entityDef )
	, m_map( map )
{
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;

	// vel += acceleration * dt;
	m_velocity += m_linearAcceleration * deltaSeconds;
	m_linearAcceleration = Vec2( 0.f, 0.f );
	// pos += vel * dt;
	m_position += m_velocity * deltaSeconds;

	//update orientation
	m_orientationDegrees += m_angularVelocity * deltaSeconds;

	ApplyFriction();

	ZephyrEntity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	if ( m_isPossessed )
	{
		return;
	}

	std::vector<Vertex_PCU> vertices;
	Vec3 corners[4];

	switch ( m_entityDef.m_billboardStyle )
	{
		case eBillboardStyle::CAMERA_FACING_XY:		BillboardSpriteCameraFacingXY( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_OPPOSING_XY:	BillboardSpriteCameraOpposingXY( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_FACING_XYZ:	BillboardSpriteCameraFacingXYZ( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_OPPOSING_XYZ:	BillboardSpriteCameraOpposingXYZ( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners ); break;
		
		default: BillboardSpriteCameraFacingXY( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners ); break;
	}
	
	Vec2 mins, maxs;
	SpriteAnimationSetDefinition* walkAnimSetDef = m_entityDef.GetSpriteAnimSetDef( "Walk" );
	SpriteAnimDefinition* walkAnimDef = nullptr;
	if ( walkAnimSetDef != nullptr )
	{
		walkAnimDef = walkAnimSetDef->GetSpriteAnimationDefForDirection( m_position, m_orientationDegrees, *g_game->GetWorldCamera() );
	}

	if ( walkAnimDef == nullptr )
	{
		AppendVertsForQuad( vertices, corners, Rgba8::WHITE );

		g_renderer->BindDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/test.png" ) );
	}
	else
	{
		const SpriteDefinition& spriteDef = walkAnimDef->GetSpriteDefAtTime( m_cumulativeTime );
		spriteDef.GetUVs( mins, maxs );

		AppendVertsForQuad( vertices, corners, Rgba8::WHITE, mins, maxs );

		g_renderer->BindDiffuseTexture( &( spriteDef.GetTexture() ) );
	}

	g_renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	if ( IsDead() )
	{
		return;
	}

	m_isDead = true;

	ZephyrEntity::Die();
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	if ( m_isPossessed )
	{
		return;
	}

	DebugAddWorldWireCylinder( Vec3( m_position, 0.f ), Vec3( m_position, m_entityDef.m_height ), m_entityDef.m_physicsRadius, Rgba8::CYAN );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
void Entity::RotateDegrees( float pitchDegrees, float yawDegrees, float rollDegrees )
{
	UNUSED( pitchDegrees );
	UNUSED( rollDegrees );

	m_orientationDegrees += yawDegrees;
}


//-----------------------------------------------------------------------------------------------
void Entity::TakeDamage( int damage )
{
	m_curHealth -= damage;
	if ( m_curHealth <= 0 )
	{
		Die();
	}
	
	g_game->AddScreenShakeIntensity(.05f);
}


//-----------------------------------------------------------------------------------------------
void Entity::ApplyFriction()
{
	if ( m_velocity.GetLength() > PHYSICS_FRICTION_FRACTION )
	{
		m_velocity -= m_velocity * PHYSICS_FRICTION_FRACTION;
	}
	else
	{
		m_velocity = Vec2( 0.f, 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName )
{
	char keyCode = GetKeyCodeFromString( keyCodeStr );

	if ( keyCode == '\0' )
	{
		return;
	}

	m_registeredKeyEvents[keyCode].push_back( eventName );
}


//-----------------------------------------------------------------------------------------------
void Entity::UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName )
{
	char keyCode = GetKeyCodeFromString( keyCodeStr );

	if ( keyCode == '\0' )
	{
		return;
	}

	auto eventIter = m_registeredKeyEvents.find( keyCode );
	if ( eventIter == m_registeredKeyEvents.end() )
	{
		return;
	}

	Strings& eventNames = eventIter->second;
	int eventIdx = 0;
	for ( ; eventIdx < (int)eventNames.size(); ++eventIdx )
	{
		if ( eventName == eventNames[eventIdx] )
		{
			break;
		}
	}

	// Remove bound event
	if ( eventIdx < (int)eventNames.size() )
	{
		eventNames.erase( eventNames.begin() + eventIdx );
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue Entity::GetGlobalVariable( const std::string& varName )
{
	if ( !IsScriptValid() )
	{
		return ZephyrValue( ERROR_ZEPHYR_VAL );
	}

	// First check c++ built in vars
	if ( varName == "id" ) { return ZephyrValue( (float)GetId() ); }
	if ( varName == "name" ) { return ZephyrValue( GetName() ); }
	if ( varName == "health" ) { return ZephyrValue( (float)m_curHealth ); }
	//if ( varName == "maxHealth" ) { return ZephyrValue( (float)m_entityDef.GetMaxHealth() ); }
	if ( varName == "position" ) { return ZephyrValue( GetPosition() ); }
	if ( varName == "forwardVec" ) { return ZephyrValue( GetForwardVector() ); }

	return ZephyrEntity::GetGlobalVariable( varName );
}


//-----------------------------------------------------------------------------------------------
void Entity::SetGlobalVariable( const std::string& varName, const ZephyrValue& value )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	// First check c++ built in vars
	if ( varName == "health" )
	{
		m_curHealth = (int)value.GetAsNumber();
		if ( m_curHealth < 0 )
		{
			Die();
		}

		return;
	}

	ZephyrEntity::SetGlobalVariable( varName, value );
}


//-----------------------------------------------------------------------------------------------
void Entity::AddGameEventParams( EventArgs* args ) const
{
	UNUSED( args );
}


//-----------------------------------------------------------------------------------------------
char Entity::GetKeyCodeFromString( const std::string& keyCodeStr )
{
	if (		IsEqualIgnoreCase( keyCodeStr, "space" ) )	{ return KEY_SPACEBAR; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "enter" ) )	{ return KEY_ENTER; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "shift" ) )	{ return KEY_SHIFT; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "left" ) )	{ return KEY_LEFTARROW; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "right" ) )	{ return KEY_RIGHTARROW; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "up" ) )		{ return KEY_UPARROW; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "down" ) )	{ return KEY_DOWNARROW; }
	else
	{
		// Register letters and numbers
		char key = keyCodeStr[0];
		if ( key >= 'a' && key <= 'z' )
		{
			key -= 32;
		}

		if ( ( key >= '0' && key <= '9' )
			 || ( key >= 'A' && key <= 'Z' ) )
		{
			return key;
		}
	}

	return '\0';
}


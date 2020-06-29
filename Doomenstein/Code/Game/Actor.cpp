#include "Game/Actor.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor( const EntityDefinition& entityDef )
	: Entity( entityDef )
{
	m_canBePushedByWalls = true;
	m_canBePushedByEntities = true;
	m_canPushEntities = true;
}


//-----------------------------------------------------------------------------------------------
void Actor::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;
		
	//UpdateAnimation();

	Entity::Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void Actor::Render() const
{
	std::vector<Vertex_PCU> vertices;
	Vec3 corners[4];

	BillboardSpriteCameraFacingXY( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );
	//BillboardSpriteCameraOpposingXY( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );
	//BillboardSpriteCameraFacingXYZ( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );
	//BillboardSpriteCameraOpposingXYZ( m_position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );
	
	

	std::map< std::string, SpriteAnimationSetDefinition* > animSetDefs = m_entityDef.GetSpriteAnimSetDefs();

	Vec2 mins, maxs;
	auto walkAnim = animSetDefs["Walk"]->GetSpriteAnimationDefForDirection( m_position, m_orientationDegrees, *g_game->GetWorldCamera() );
	if ( walkAnim == nullptr )
	{
		AppendVertsForQuad( vertices, corners, Rgba8::WHITE );

		g_renderer->BindDiffuseTexture( g_renderer->CreateOrGetTextureFromFile( "Data/Images/test.png" ) );
	}
	else
	{
		const SpriteDefinition& spriteDef = walkAnim->GetSpriteDefAtTime( m_cumulativeTime );
		spriteDef.GetUVs( mins, maxs );

		AppendVertsForQuad( vertices, corners, Rgba8::WHITE, mins, maxs );

		g_renderer->BindDiffuseTexture( &( spriteDef.GetTexture() ) );
	}

	g_renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void Actor::Die()
{
	Entity::Die();
}

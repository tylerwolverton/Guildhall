#include "Game/EnemySquare.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Enemy.hpp"


//-----------------------------------------------------------------------------------------------
EnemySquare::EnemySquare( Game* game, float sizeModifier, Vec2 position, const Rgba8& color )
	: Entity( game, position )
{
	m_entityType = EntityType::ENTITY_TYPE_BOX;

	m_mainColor = color;

	m_speed *= m_game->m_randNumGen->GetRandomFloatInRange( 0.5f, 3.f );

	/*Vec2 bottomLeft( m_game->m_randNumGen->GetRandomFloatInRange( -15.f, 0.f ), m_game->m_randNumGen->GetRandomFloatInRange( -15.f, 0.f ) );
	Vec2 topRight( m_game->m_randNumGen->GetRandomFloatInRange( 0.f, 15.f ), m_game->m_randNumGen->GetRandomFloatInRange( 0.f, 15.f ) );*/
	Vec2 bottomLeft(-4.f, -4.f);
	Vec2 topRight(4.f, 4.f);

	//AABB2 box( bottomLeft * sizeModifier, topRight * sizeModifier );
	AABB2 box( bottomLeft * sizeModifier, topRight * sizeModifier );
	m_physicsBox = box;
	m_physicsRadius = 0.f;

	m_vertexes.push_back( Vertex_PCU( box.minPoint, m_mainColor ) );
	m_vertexes.push_back( Vertex_PCU( Vec2( box.maxPoint.x, box.minPoint.y ), m_mainColor ) );
	m_vertexes.push_back( Vertex_PCU( box.maxPoint, m_mainColor ) );

	m_vertexes.push_back( Vertex_PCU( box.minPoint, m_mainColor ) );
	m_vertexes.push_back( Vertex_PCU( box.maxPoint, m_mainColor ) );
	m_vertexes.push_back( Vertex_PCU( Vec2( box.minPoint.x, box.maxPoint.y ), m_mainColor ) );

	//m_health = 5;
}


//-----------------------------------------------------------------------------------------------
EnemySquare::~EnemySquare()
{
}


//-----------------------------------------------------------------------------------------------
void EnemySquare::Update( float deltaSeconds )
{
	for ( int attackerIndex = 0; attackerIndex < (int)m_attackers.size(); ++attackerIndex )
	{
		TakeDamage( 1 );
	}

	m_moveDirection = GetNormalizedDirectionFromAToB( m_position, m_game->GetPlayer()->GetPosition() );

	m_velocity = ENEMY_SPEED * m_moveDirection;

	Entity::Update( deltaSeconds );

	CheckIfOutOfBounds();

	m_vertexesCopy = m_vertexes;
	if ( (int)m_vertexesCopy.size() > 0 )
	{
		Vertex_PCU::TransformVertexArray( m_vertexesCopy, 1.f, m_orientationDegrees, m_position );
	}

	Vec2 minToMax = m_physicsBox.maxPoint - m_physicsBox.minPoint;
	m_physicsBox.minPoint = m_position - minToMax * .5f;
	m_physicsBox.maxPoint = m_position + minToMax * .5f;
}


//-----------------------------------------------------------------------------------------------
void EnemySquare::CheckIfOutOfBounds()
{
	if ( m_position.x > WORLD_SIZE_X
		 || m_position.x < 0.f 
		 || m_position.y > WORLD_SIZE_Y
		 || m_position.y < 0.f )
	{
		m_orientationDegrees += 180.f;
	}
}


//-----------------------------------------------------------------------------------------------
void EnemySquare::AttachAttacker( Entity* attacker )
{
	m_attackers.push_back( attacker );

	//AddVertexes( attacker->GetVertexes() );
}


//-----------------------------------------------------------------------------------------------
void EnemySquare::AddVertexes( std::vector<Vertex_PCU> vertexes )
{
	for ( int vertIndex = 0; vertIndex < (int)vertexes.size(); ++vertIndex )
	{
		m_vertexes.push_back( vertexes[vertIndex] );
	}
}


//-----------------------------------------------------------------------------------------------
void EnemySquare::Render() const
{
	if ( IsDead() )
	{
		return;
	}

	g_theRenderer->DrawVertexArray( m_vertexesCopy );
}


//-----------------------------------------------------------------------------------------------
void EnemySquare::Die()
{
	m_isGarbage = true;

	m_game->AddScreenShakeIntensity( .15f );

	Entity::Die();
	
	float explodeDirMod = 1.f;
	for ( int vertexIndex = 0; vertexIndex < (int)m_vertexes.size(); )
	{
		std::vector<Vertex_PCU> newVertices;
		newVertices.push_back( m_vertexes[vertexIndex] );
		newVertices.push_back( m_vertexes[vertexIndex + 1] );
		newVertices.push_back( m_vertexes[vertexIndex + 2] );

		float newRotDegrees = m_game->m_randNumGen->GetRandomFloatInRange( -20.f, 20.f );
		newVertices[0].m_position = newVertices[0].m_position.GetRotatedAboutZDegrees( newRotDegrees );
		newVertices[1].m_position = newVertices[1].m_position.GetRotatedAboutZDegrees( newRotDegrees );
		newVertices[1].m_position = newVertices[1].m_position.GetRotatedAboutZDegrees( newRotDegrees );

		m_game->SpawnEnemy( newVertices, m_mainColor, m_position + ( m_physicsRadius + m_game->m_randNumGen->GetRandomFloatInRange(3.f, 20.f) ) * GetForwardVector(), explodeDirMod * 15.f * GetForwardVector() );

		explodeDirMod *= -1.f;
		vertexIndex += 3;
	}
}


//-----------------------------------------------------------------------------------------------
void EnemySquare::Collide( const Entity* otherEntity )
{
	//otherEntity
}


//-----------------------------------------------------------------------------------------------
void EnemySquare::DebugRender() const
{
	g_theRenderer->DrawAABB2( m_physicsBox, Rgba8( 0, 255, 255 ), DEBUG_LINE_THICKNESS );
}

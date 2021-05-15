#include "Game/ConvexRaycastMap.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/BufferParser.hpp"
#include "Engine/Core/BufferWriter.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Time/Time.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/SymmetricQuadTree.hpp"


//-----------------------------------------------------------------------------------------------
ConvexRaycastMap::ConvexRaycastMap( const std::string& name )
	: Map( name )
{
	m_symmetricQuadTree = new SymmetricQuadTree( AABB2( 0.f, 0.f, g_windowWidth, g_windowHeight ), 3 );

	AddEntitySet( NUM_ENTITIES_PER_SET );
}


//-----------------------------------------------------------------------------------------------
ConvexRaycastMap::~ConvexRaycastMap()
{
	PTR_SAFE_DELETE( m_symmetricQuadTree );
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::Reset()
{
	m_draggedEntity = nullptr;
	m_symmetricQuadTree->RemoveEntitiesAboveId( -1 );
	m_numActiveEntities = 0;
	m_numTotalEntities = 0;
	Map::Reset();

	AddEntitySet( NUM_ENTITIES_PER_SET );
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::Update( float deltaSeconds )
{
	UpdateFromKeyboard( deltaSeconds );
	UpdateEntities( deltaSeconds );

	UpdateHighlightedEntity();
	UpdateDraggedEntity();

	UpdateVisibleRaycast();
	UpdateInvisibleRaycasts();

	m_numRaycastImpacts = 0;
	m_curRayNum = 0;
	double timeBeforeRaycasts = GetCurrentTimeSeconds();

	PerformInvisibleRaycasts();

	m_curRayNum = 9999998; // special marker to signal visible for debug draw purposes
	PerformVisibleRaycast();

	m_raycastTimeMs = ( GetCurrentTimeSeconds() - timeBeforeRaycasts ) * 1000.0;
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::Render() const
{
	std::vector<Vertex_PCU> vertices;

	for ( int entityIdx = 0; entityIdx < m_numActiveEntities; ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Render( vertices );
	}

	RenderVisibleRaycast( vertices );

	g_renderer->BindDiffuseTexture( nullptr );
	g_renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::DebugRender() const
{
	std::vector<Vertex_PCU> vertices;

	for ( int entityIdx = 0; entityIdx < m_numActiveEntities; ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->DebugRender( vertices );
	}

	m_symmetricQuadTree->DebugRender( vertices );
	m_symmetricQuadTree->DebugRenderRaycast( vertices, m_visibleRaycastStartPos, m_visibleRaycastForwardVector, m_visibleRaycastResult.maxDist );

	g_renderer->BindDiffuseTexture( nullptr );
	g_renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::AddEntitySet( int numNewEntites )
{
	// Check if we've already created enough to satisfy request
	if ( m_numTotalEntities - m_numActiveEntities > numNewEntites )
	{
		// Add the new entities to the quad tree, add 1 since the constructor is max exclusive
		std::vector<Entity*> entitiesToAddToQuadTree( &m_entities[m_numActiveEntities], &m_entities[m_numActiveEntities+ numNewEntites - 1] + 1 );
		m_symmetricQuadTree->AddEntities( entitiesToAddToQuadTree );

		m_numActiveEntities += numNewEntites;

		return;
	}

	SpawnNewEntitySet( numNewEntites );

	// Add the new entities to the quad tree, add 1 since the constructor is max exclusive
	std::vector<Entity*> entitiesToAddToQuadTree( &m_entities[m_numActiveEntities], &m_entities[m_numActiveEntities + numNewEntites - 1] + 1 );
	m_symmetricQuadTree->AddEntities( entitiesToAddToQuadTree );
	
	m_numActiveEntities += numNewEntites;

	if ( m_numActiveEntities > m_numTotalEntities )
	{
		m_numTotalEntities = m_numActiveEntities;
	}
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::RemoveEntitySet( int numEntitesToRemove )
{
	if ( m_numActiveEntities - numEntitesToRemove < 1 )
	{
		m_symmetricQuadTree->RemoveEntitiesAboveId( 0 );
		m_numActiveEntities = 1;
		return;
	}

	m_numActiveEntities -= numEntitesToRemove;

	m_symmetricQuadTree->RemoveEntitiesAboveId( m_numActiveEntities - 1 );
}


//-----------------------------------------------------------------------------------------------
int ConvexRaycastMap::GetObjectCount() const
{
	return m_numActiveEntities;
}


//-----------------------------------------------------------------------------------------------
int ConvexRaycastMap::GetRaycastCount() const
{
	return m_numInvisibleRaycasts;
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::SpawnNewEntitySet( int numNewEntites )
{
	for ( int entityNum = 0; entityNum < numNewEntites; ++entityNum )
	{
		float radius = g_game->m_rng->RollRandomFloatInRange( .1f, 1.f ) * g_windowWidth * .075f;
		float xPos = g_game->m_rng->RollRandomFloatInRange( g_windowWidth * .075f, g_windowWidth - ( g_windowWidth * .075f ) );
		float yPos = g_game->m_rng->RollRandomFloatInRange( g_windowHeight * .075f, g_windowHeight - ( g_windowHeight * .075f ) );

		Vec2 center( xPos, yPos );

		std::vector<Vec2> polygonPoints;
		for ( int curDegrees = 0; curDegrees < 360; curDegrees += g_game->m_rng->RollRandomIntInRange( 30, 140 ) )
		{
			Vec2 vectorToStartingPoint( 1.f, 0.f );
			polygonPoints.push_back( radius * vectorToStartingPoint.GetRotatedDegrees( (float)curDegrees ) );
		}

		Entity* newEntity = SpawnNewEntity( Polygon2( polygonPoints ) );
		newEntity->UpdatePosition( center );
		newEntity->SetPhysicsRadius( radius );
	}
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::AddInvisibleRaycasts( int numNewRaycasts )
{
	m_numInvisibleRaycasts += numNewRaycasts;
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::RemoveInvisibleRaycasts( int numRaycastsToRemove )
{
	m_numInvisibleRaycasts -= numRaycastsToRemove;
	if ( m_numInvisibleRaycasts < 0 )
	{
		m_numInvisibleRaycasts = 0;
	}
}


//-----------------------------------------------------------------------------------------------
RaycastResult ConvexRaycastMap::Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist )
{
	++m_curRayNum;

	if ( m_broadphaseCheckType == eBroadphaseCheckType::QUAD_TREE )
	{
		return RaycastWithQuadTree( startPos, forwardNormal, maxDist );
	}

	RaycastResult closestResult;
	closestResult.startPos = startPos;
	closestResult.forwardNormal = forwardNormal;
	closestResult.maxDist = maxDist;
	closestResult.impactDist = maxDist;
	closestResult.impactPos = startPos + ( forwardNormal * maxDist );

	for ( int entityIdx = 0; entityIdx < m_numActiveEntities; ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}
		
		if ( entity->IsPositionInside( startPos ) )
		{
			++m_numRaycastImpacts;

			closestResult.didImpact = true;
			closestResult.impactDist = 0.f;
			closestResult.impactEntity = entity;
			closestResult.impactFraction = 0.f;
			closestResult.impactPos = startPos;
			closestResult.impactSurfaceNormal = -forwardNormal;

			return closestResult;
		}

		// Return false if ray is length zero and not in an object
		if ( IsNearlyEqual( startPos, forwardNormal * maxDist ) )
		{
			return closestResult;
		}

		// Check narrow phase
		if ( m_narrowphaseCheckType == eNarrowphaseCheckType::BOUNDING_DISC )
		{
			if ( !DoLineSegmentAndDiscOverlap2D( startPos, forwardNormal, maxDist, entity->GetPosition(), entity->GetPhysicsRadius() ) )
			{
				continue;
			}
		}

		RayConvexHullIntersectionResult entityRaycastResult = entity->GetRayConvexHullIntersectionResult( startPos, forwardNormal );
		// Returns start position if no intersection occurs
		if ( IsNearlyEqual( entityRaycastResult.intersectionPoint, startPos ) )
		{
			continue;
		}

		float impactDist = GetDistance2D( startPos, entityRaycastResult.intersectionPoint );
		if ( impactDist < closestResult.impactDist )
		{
			// We found an entity
			closestResult.didImpact = true;
			closestResult.impactDist = impactDist;
			closestResult.impactEntity = entity;
			closestResult.impactFraction = impactDist / maxDist;
			closestResult.impactPos = entityRaycastResult.intersectionPoint;
			closestResult.impactSurfaceNormal = entityRaycastResult.surfaceNormal;
		}
	}

	if ( closestResult.didImpact )
	{
		++m_numRaycastImpacts;
	}

	return closestResult;
}


//-----------------------------------------------------------------------------------------------
RaycastResult ConvexRaycastMap::RaycastWithQuadTree( const Vec2& startPos, const Vec2& forwardNormal, float maxDist )
{
	RaycastResult closestResult;
	closestResult.startPos = startPos;
	closestResult.forwardNormal = forwardNormal;
	closestResult.maxDist = maxDist;
	closestResult.impactDist = maxDist;
	closestResult.impactPos = startPos + ( forwardNormal * maxDist );

	std::vector<EntityId> entitiesToTest = m_symmetricQuadTree->GetEntitiesFromRaycast( startPos, forwardNormal, maxDist );

	for ( int entityIdx = 0; entityIdx < (int)entitiesToTest.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entitiesToTest[entityIdx]];
		if ( entity == nullptr
			 || entity->GetLastRayCollisionDetectedNum() == m_curRayNum )
		{
			continue;
		}

		entity->SetLastRayCollisionDetectedNum( m_curRayNum );
		entity->MarkAsCollisionTested();

		if ( entity->IsPositionInside( startPos ) )
		{
			++m_numRaycastImpacts;

			closestResult.didImpact = true;
			closestResult.impactDist = 0.f;
			closestResult.impactEntity = entity;
			closestResult.impactFraction = 0.f;
			closestResult.impactPos = startPos;
			closestResult.impactSurfaceNormal = -forwardNormal;

			return closestResult;
		}

		// Return false if ray is length zero and not in an object
		if ( IsNearlyEqual( startPos, forwardNormal * maxDist ) )
		{
			return closestResult;
		}

		// Check narrow phase
		if ( m_narrowphaseCheckType == eNarrowphaseCheckType::BOUNDING_DISC )
		{
			if ( !DoLineSegmentAndDiscOverlap2D( startPos, forwardNormal, maxDist, entity->GetPosition(), entity->GetPhysicsRadius() ) )
			{
				continue;
			}
		}

		RayConvexHullIntersectionResult entityRaycastResult = entity->GetRayConvexHullIntersectionResult( startPos, forwardNormal );
		// Returns start position if no intersection occurs
		if ( IsNearlyEqual( entityRaycastResult.intersectionPoint, startPos ) )
		{
			continue;
		}

		float impactDist = GetDistance2D( startPos, entityRaycastResult.intersectionPoint );
		if ( impactDist < closestResult.impactDist )
		{
			// We found an entity
			closestResult.didImpact = true;
			closestResult.impactDist = impactDist;
			closestResult.impactEntity = entity;
			closestResult.impactFraction = impactDist / maxDist;
			closestResult.impactPos = entityRaycastResult.intersectionPoint;
			closestResult.impactSurfaceNormal = entityRaycastResult.surfaceNormal;
		}
	}

	if ( closestResult.didImpact )
	{
		++m_numRaycastImpacts;
	}

	return closestResult;
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::UpdateFromKeyboard( float deltaSeconds )
{
	if ( g_devConsole->IsOpen() )
	{
		return;
	}

	if ( g_inputSystem->WasKeyJustPressed( MOUSE_LBUTTON ) )
	{
		m_draggedEntity = GetTopEntityAtMousePosition();
		if( m_draggedEntity != nullptr )
		{ 
			m_symmetricQuadTree->RemoveEntity( m_draggedEntity->GetId() );
		}
	}

	if ( g_inputSystem->WasKeyJustReleased( MOUSE_LBUTTON ) )
	{
		if ( m_draggedEntity != nullptr )
		{
			m_symmetricQuadTree->AddEntity( m_draggedEntity );
		}

		m_draggedEntity = nullptr;
		m_mouseOffsetFromCenterOfDraggedEntity = Vec2::ZERO;
	}

	m_isRaycastStartMoving = false;
	m_isRaycastEndMoving = false;

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		m_isRaycastStartMoving = true;
	}
	else if ( g_inputSystem->IsKeyPressed( 'E' ) )
	{
		m_isRaycastEndMoving = true;
	}

	if ( g_inputSystem->IsKeyPressed( 'W' ) )
	{
		Entity* entityToRotate = m_draggedEntity;
		if ( entityToRotate == nullptr )
		{
			entityToRotate = GetTopEntityAtMousePosition();
		}

		if ( entityToRotate == nullptr )
		{
			return;
		}

		entityToRotate->RotateAboutPoint2D( -180.f * deltaSeconds, g_game->GetMouseWorldPosition() );
		m_symmetricQuadTree->RemoveEntity( entityToRotate->GetId() );

		if ( m_draggedEntity == nullptr )
		{
			m_symmetricQuadTree->AddEntity( entityToRotate );
		}
	}
	else if ( g_inputSystem->IsKeyPressed( 'R' ) )
	{
		Entity* entityToRotate = m_draggedEntity;
		if ( entityToRotate == nullptr )
		{
			entityToRotate = GetTopEntityAtMousePosition();
		}

		if ( entityToRotate == nullptr )
		{
			return;
		}

		entityToRotate->RotateAboutPoint2D( 180.f * deltaSeconds, g_game->GetMouseWorldPosition() );
		m_symmetricQuadTree->RemoveEntity( entityToRotate->GetId() );

		if ( m_draggedEntity == nullptr )
		{
			m_symmetricQuadTree->AddEntity( entityToRotate );
		}
	}

	if ( g_inputSystem->IsKeyPressed( 'K' ) )
	{
		Entity* entityToScale = m_draggedEntity;
		if ( entityToScale == nullptr )
		{
			entityToScale = GetTopEntityAtMousePosition();
		}

		if ( entityToScale == nullptr )
		{
			return;
		}

		entityToScale->ScaleAboutPoint2D( 1.f + .25f * deltaSeconds, g_game->GetMouseWorldPosition() );
		m_symmetricQuadTree->RemoveEntity( entityToScale->GetId() );
		
		if ( m_draggedEntity == nullptr )
		{
			m_symmetricQuadTree->AddEntity( entityToScale );
		}
	}
	else if ( g_inputSystem->IsKeyPressed( 'L' ) )
	{
		Entity* entityToScale = m_draggedEntity;
		if ( entityToScale == nullptr )
		{
			entityToScale = GetTopEntityAtMousePosition();
		}

		if ( entityToScale == nullptr )
		{
			return;
		}

		entityToScale->ScaleAboutPoint2D( 1.f - .25f * deltaSeconds, g_game->GetMouseWorldPosition() );
		m_symmetricQuadTree->RemoveEntity( entityToScale->GetId() );
		
		if ( m_draggedEntity == nullptr )
		{
			m_symmetricQuadTree->AddEntity( entityToScale );
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( 'N' ) )
	{
		int numRaycastsToRemove = m_numInvisibleRaycasts / 2;
		if ( m_numInvisibleRaycasts == 1 )
		{
			numRaycastsToRemove = 1;
		}

		RemoveInvisibleRaycasts( numRaycastsToRemove );
	}

	if ( g_inputSystem->WasKeyJustPressed( 'M' ) )
	{
		int numRaycastsToAdd = m_numInvisibleRaycasts;
		if ( numRaycastsToAdd == 0 )
		{
			numRaycastsToAdd = 1;
		}

		AddInvisibleRaycasts( numRaycastsToAdd );
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_PERIOD ) )
	{		
		AddEntitySet( m_numActiveEntities );
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_COMMA ) )
	{
		int numEntitiesToRemove = m_numActiveEntities / 2;
		if ( numEntitiesToRemove == 0 )
		{
			numEntitiesToRemove = 1;
		}

		RemoveEntitySet( numEntitiesToRemove );
	}
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::UpdateHighlightedEntity()
{
	if ( m_draggedEntity != nullptr )
	{
		m_draggedEntity->Highlight();
		return;
	}

	Entity* topEntity = GetTopEntityAtMousePosition();

	// Highlight new entity hovered
	if ( m_hoverEntity == nullptr 
		 && topEntity != nullptr )
	{
		m_hoverEntity = topEntity;
		m_hoverEntity->Highlight();
	}

	// UnHighlight old hovered and highlight new if one exists
	if ( m_hoverEntity != nullptr
		 && topEntity != m_hoverEntity )
	{
		m_hoverEntity->UnHighlight();
		m_hoverEntity = topEntity;

		if ( m_hoverEntity != nullptr )
		{
			m_hoverEntity->Highlight();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::UpdateDraggedEntity()
{
	if ( m_draggedEntity != nullptr )
	{
		m_draggedEntity->UpdatePosition( g_game->GetMouseWorldPosition() + m_mouseOffsetFromCenterOfDraggedEntity );
	}
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::UpdateEntities( float deltaSeconds )
{
	for ( int entityIdx = 0; entityIdx < m_numActiveEntities; ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Update( deltaSeconds );
	}
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::UpdateVisibleRaycast()
{
	if ( m_isRaycastStartMoving )
	{
		m_visibleRaycastStartPos = g_game->GetMouseWorldPosition();
		m_visibleRaycastForwardVector = ( m_visibleRaycastEndPos - m_visibleRaycastStartPos ).GetNormalized();
	}
	
	if ( m_isRaycastEndMoving )
	{
		m_visibleRaycastEndPos = g_game->GetMouseWorldPosition();
		m_visibleRaycastForwardVector = ( m_visibleRaycastEndPos - m_visibleRaycastStartPos ).GetNormalized();
	}

	m_visibleRaycastMaxDist = GetDistance2D( m_visibleRaycastStartPos, m_visibleRaycastEndPos );
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::UpdateInvisibleRaycasts()
{
	m_invisibleRays.clear();

	for ( int raycastNum = 0; raycastNum < m_numInvisibleRaycasts; ++raycastNum )
	{
		Vec2 startPos = Vec2( g_game->m_rng->RollRandomFloatInRange( 0.f, g_windowWidth ),
							  g_game->m_rng->RollRandomFloatInRange( 0.f, g_windowHeight ) );

		Vec2 forwardNormal( 1.f, 0.f );
		forwardNormal.RotateDegrees( g_game->m_rng->RollRandomFloatInRange( 0.f, 360.f ) );

		float maxDist = g_game->m_rng->RollRandomFloatInRange( 1.f, 10.f );

		Ray ray;
		ray.startPos = startPos;
		ray.forwardNormal = forwardNormal;
		ray.maxDist = maxDist;

		m_invisibleRays.push_back( ray );
	}
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::PerformInvisibleRaycasts()
{
	for ( int raycastIdx = 0; raycastIdx < (int)m_invisibleRays.size(); ++raycastIdx )
	{
		RaycastResult result =  Raycast( m_invisibleRays[raycastIdx].startPos, m_invisibleRays[raycastIdx].forwardNormal, m_invisibleRays[raycastIdx].maxDist );
		m_dummyVal = result.impactDist;
	}
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::PerformVisibleRaycast()
{
	m_visibleRaycastResult = Raycast( m_visibleRaycastStartPos, m_visibleRaycastForwardVector, m_visibleRaycastMaxDist );
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::RenderVisibleRaycast( std::vector<Vertex_PCU>& vertices ) const
{
	AppendVertsForLine2D(vertices, m_visibleRaycastStartPos, m_visibleRaycastEndPos, Rgba8::GREY, g_debugLineThickness );
	AppendVertsForLine2D(vertices, m_visibleRaycastStartPos, m_visibleRaycastResult.impactPos, Rgba8::RED, g_debugLineThickness );
	AppendVertsForDisc2D(vertices, m_visibleRaycastStartPos, g_debugLineThickness, Rgba8::CYAN );
	AppendVertsForDisc2D(vertices, m_visibleRaycastResult.impactPos, g_debugLineThickness, Rgba8::MAGENTA );

	if ( m_visibleRaycastResult.didImpact )
	{
		AppendVertsForLine2D( vertices, m_visibleRaycastResult.impactPos, m_visibleRaycastResult.impactPos + ( m_visibleRaycastResult.impactSurfaceNormal ) * g_debugLineThickness * 20.f, Rgba8::PURPLE, g_debugLineThickness );
	}
}


//-----------------------------------------------------------------------------------------------
Entity* ConvexRaycastMap::GetTopEntityAtMousePosition()
{
	Vec2 mousePosition = g_game->GetMouseWorldPosition();

	Entity* topEntity = GetTopEntityAtPosition( mousePosition );
	if ( topEntity == nullptr )
	{
		return nullptr;
	}

	m_mouseOffsetFromCenterOfDraggedEntity = topEntity->GetPosition() - mousePosition;

	return topEntity;
}


//-----------------------------------------------------------------------------------------------
Entity* ConvexRaycastMap::GetTopEntityAtPosition( const Vec2& position ) const
{
	// Walk the entities backwards to find topmost entities first
	for ( int entityNum = m_numActiveEntities - 1; entityNum >= 0; --entityNum )
	{
		if ( m_entities[entityNum]->IsPositionInside( position ) )
		{
			return m_entities[entityNum];
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::SaveConvexSceneToFile( const std::string& fileName )
{
	std::vector<byte> buffer;
	BufferWriter bufferWriter( buffer );
	//bufferWriter.SetEndianMode( eBufferEndianMode::BIG );

	WriteFileHeaderToBuffer( bufferWriter );

	std::vector<byte> tocBuffer;
	BufferWriter tocBufferWriter( tocBuffer );
	tocBufferWriter.SetEndianMode( bufferWriter.GetEndianMode() );
	tocBufferWriter.AppendByte( 0 );

	WriteConvexPolysChunk( bufferWriter, tocBuffer );
	WriteSceneInfoChunk( bufferWriter, tocBuffer );
	WriteBoundingDiscsChunk( bufferWriter, tocBuffer );

	WriteTableOfContentsToBuffer( bufferWriter, tocBuffer );

	if ( !WriteBufferToFile( "Data/" + fileName, buffer.data(), (uint32_t)buffer.size() ) )
	{
		g_devConsole->PrintError( "Failed to write out convex scene file" );
	}
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::WriteFileHeaderToBuffer( BufferWriter& bufferWriter )
{
	bufferWriter.AppendChar( 'G' );
	bufferWriter.AppendChar( 'H' );
	bufferWriter.AppendChar( 'C' );
	bufferWriter.AppendChar( 'S' );
	bufferWriter.AppendByte( 0 );
	bufferWriter.AppendByte( 29 );
	bufferWriter.AppendByte( 1 );
	bufferWriter.AppendByte( (byte)bufferWriter.GetEndianMode() );
	bufferWriter.AppendUint32( 0 );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::WriteConvexPolysChunk( BufferWriter& bufferWriter, std::vector<byte>& tocBuffer )
{
	// Increase chunk count
	++tocBuffer[0];

	// Save location in table of contents
	BufferWriter tocBufferWriter( tocBuffer );
	tocBufferWriter.SetEndianMode( bufferWriter.GetEndianMode() );

	tocBufferWriter.AppendByte( 1 ); // chunk type
	uint32_t chunkHeaderStartOffset = bufferWriter.GetBufferLength();
	tocBufferWriter.AppendUint32( chunkHeaderStartOffset );

	// Write chunk
	bufferWriter.AppendChar( '\0' );
	bufferWriter.AppendChar( 'C' );
	bufferWriter.AppendChar( 'H' );
	bufferWriter.AppendChar( 'K' );
	bufferWriter.AppendByte( 1 ); // chunk type
	bufferWriter.AppendByte( 1 ); // little endian

	eBufferEndianMode curBufferEndianMode = bufferWriter.GetEndianMode();
	bufferWriter.SetEndianMode( eBufferEndianMode::LITTLE );

	bufferWriter.AppendUint32( 0 ); // placeholder for size

	uint32_t chunkDataStartOffset = bufferWriter.GetBufferLength();

	// Write out polys
	bufferWriter.AppendUint32( (uint32_t)m_numActiveEntities ); // total num polys

	for ( int polyIdx = 0; polyIdx < m_numActiveEntities; ++ polyIdx )
	{
		if ( m_entities[polyIdx] == nullptr )
		{
			bufferWriter.AppendUshort( (uint16_t)0 );
			continue;
		}

		Polygon2 const& convexPolygon = m_entities[polyIdx]->GetConvexPolygon();

		int numVerts = convexPolygon.GetVertexCount();
		bufferWriter.AppendUshort( (uint16_t)numVerts );

		for ( int vertIdx = 0; vertIdx < numVerts; ++vertIdx )
		{
			bufferWriter.AppendVec2( convexPolygon.GetPoints()[vertIdx] );
		}
	}

	// Update chunk size in both chunk and table of contents
	uint32_t chunkSize = bufferWriter.GetBufferLength() - chunkDataStartOffset;
	bufferWriter.OverwriteUint32AtOffset( chunkSize, chunkHeaderStartOffset + 6 );
	tocBufferWriter.AppendUint32( chunkSize );

	bufferWriter.SetEndianMode( curBufferEndianMode );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::WriteSceneInfoChunk( BufferWriter& bufferWriter, std::vector<byte>& tocBuffer )
{
	// Increase chunk count
	++tocBuffer[0];

	// Save location in table of contents
	BufferWriter tocBufferWriter( tocBuffer );
	tocBufferWriter.SetEndianMode( bufferWriter.GetEndianMode() );

	tocBufferWriter.AppendByte( 128 ); // chunk type
	uint32_t chunkHeaderStartOffset = bufferWriter.GetBufferLength();
	tocBufferWriter.AppendUint32( chunkHeaderStartOffset );

	// Write chunk
	bufferWriter.AppendChar( '\0' );
	bufferWriter.AppendChar( 'C' );
	bufferWriter.AppendChar( 'H' );
	bufferWriter.AppendChar( 'K' );
	bufferWriter.AppendByte( 128 ); // chunk type
	bufferWriter.AppendByte( 1 ); // little endian

	eBufferEndianMode curBufferEndianMode = bufferWriter.GetEndianMode();
	bufferWriter.SetEndianMode( eBufferEndianMode::LITTLE );

	bufferWriter.AppendUint32( (uint32_t)16 ); // size
	tocBufferWriter.AppendUint32( (uint32_t)16 );
	
	// Write out scene size
	bufferWriter.AppendAABB2( AABB2( 0.f, 0.f, g_windowWidth, g_windowHeight ) );

	bufferWriter.SetEndianMode( curBufferEndianMode );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::WriteBoundingDiscsChunk( BufferWriter& bufferWriter, std::vector<byte>& tocBuffer )
{
	// Increase chunk count
	++tocBuffer[0];

	// Save location in table of contents
	BufferWriter tocBufferWriter( tocBuffer );
	tocBufferWriter.SetEndianMode( bufferWriter.GetEndianMode() );

	tocBufferWriter.AppendByte( 129 ); // chunk type
	uint32_t chunkHeaderStartOffset = bufferWriter.GetBufferLength();
	tocBufferWriter.AppendUint32( chunkHeaderStartOffset );

	// Write chunk
	bufferWriter.AppendChar( '\0' );
	bufferWriter.AppendChar( 'C' );
	bufferWriter.AppendChar( 'H' );
	bufferWriter.AppendChar( 'K' );
	bufferWriter.AppendByte( 129 ); // chunk type
	bufferWriter.AppendByte( 1 ); // little endian

	eBufferEndianMode curBufferEndianMode = bufferWriter.GetEndianMode();
	bufferWriter.SetEndianMode( eBufferEndianMode::LITTLE );

	uint32_t chunkSize = 4 + ( 12 * m_numActiveEntities );
	bufferWriter.AppendUint32( chunkSize );
	tocBufferWriter.AppendUint32( chunkSize );

	// Num entities
	bufferWriter.AppendUint32( (uint32_t)m_numActiveEntities );

	for ( int polyIdx = 0; polyIdx < m_numActiveEntities; ++polyIdx )
	{
		if ( m_entities[polyIdx] == nullptr )
		{
			bufferWriter.AppendVec2( Vec2::ZERO );
			bufferWriter.AppendFloat( 0.f );
			continue;
		}

		Entity* const& entity = m_entities[polyIdx];

		bufferWriter.AppendVec2( entity->GetPosition() );
		bufferWriter.AppendFloat( entity->GetPhysicsRadius() );
	}

	bufferWriter.SetEndianMode( curBufferEndianMode );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::WriteTableOfContentsToBuffer( BufferWriter& bufferWriter, std::vector<byte>& tocBuffer )
{
	// Update offset to toc in header
	uint32_t tocOffset = bufferWriter.GetBufferLength();
	bufferWriter.OverwriteInt32AtOffset( tocOffset, 8 );

	bufferWriter.AppendChar( '\0' );
	bufferWriter.AppendChar( 'T' );
	bufferWriter.AppendChar( 'O' );
	bufferWriter.AppendChar( 'C' );

	BufferParser tocParser( tocBuffer );
	tocParser.SetEndianMode( bufferWriter.GetEndianMode() );

	byte numChunks = tocParser.ParseByte();
	bufferWriter.AppendByte( numChunks );

	for ( byte chunkIdx = 0; chunkIdx < numChunks; ++chunkIdx )
	{
		bufferWriter.AppendByte( tocParser.ParseByte() );
		bufferWriter.AppendUint32( tocParser.ParseUint32() );
		bufferWriter.AppendUint32( tocParser.ParseUint32() );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
void ConvexRaycastMap::LoadConvexSceneFromFile( const std::string& fileName )
{
	uint32_t fileSize;
	byte* buffer = (byte*)FileReadBinaryToNewBuffer( "Data/" + fileName, &fileSize );
	
	if ( buffer == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Could not read in ghcs file '%s'. Ensure that file is present in Run/Data/", fileName.c_str() ) );
		return;
	}

	BufferParser bufferParser( buffer, fileSize );
	
	uint32_t offsetToTOC = 0;
	if ( !ReadFileHeaderFromBuffer( bufferParser, offsetToTOC ) ) { return; }
	if ( !ReadChunksFromBuffer( bufferParser, offsetToTOC ) ) { return; }
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::ReadFileHeaderFromBuffer( BufferParser& bufferParser, uint32_t& out_offsetToTOC )
{
	if ( bufferParser.ParseChar() != 'G' 
		 || bufferParser.ParseChar() != 'H' 
		 || bufferParser.ParseChar() != 'C' 
		 || bufferParser.ParseChar() != 'S' )
	{
		g_devConsole->PrintError( "Invalid File: Header CC is not GHCS" );
		return false;
	}

	if ( bufferParser.ParseByte() != 0 )
	{
		g_devConsole->PrintError( "Invalid File: Missing reserved byte in header" );
		return false;
	}

	// Temp checks for testing, will handle different revisions later
	byte majorVersion = bufferParser.ParseByte();
	if ( majorVersion != 29 )
	{
		g_devConsole->PrintError( "Invalid File: Expected major version 29" );
		return false;
	}

	byte minorVersion = bufferParser.ParseByte();
	if ( minorVersion != 1 )
	{
		g_devConsole->PrintError( "Invalid File: Expected minor version 1" );
		return false;
	}

	byte endianMode = bufferParser.ParseByte();
	switch ( endianMode )
	{
		case 1: bufferParser.SetEndianMode( eBufferEndianMode::LITTLE ); break;
		case 2: bufferParser.SetEndianMode( eBufferEndianMode::BIG ); break;
		default:
		{
			g_devConsole->PrintError( "Invalid File: Unsupported endian mode" );
			return false;
		}
	}

	out_offsetToTOC = bufferParser.ParseUint32();

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::ReadChunksFromBuffer( BufferParser& bufferParser, uint32_t offsetToTOC )
{
	eBufferEndianMode tocEndianMode = bufferParser.GetEndianMode();

	constexpr byte TOC_HEADER_SIZE = 5;
	bufferParser.SetReadOffset( offsetToTOC );

	if ( bufferParser.ParseChar() != '\0'
		 || bufferParser.ParseChar() != 'T'
		 || bufferParser.ParseChar() != 'O'
		 || bufferParser.ParseChar() != 'C' )
	{
		g_devConsole->PrintError( "Invalid File: TOC CC is not 0TOC" );
		return false;
	}

	byte numChunks = bufferParser.ParseByte();

	constexpr byte TOC_CHUNK_ENTRY_SIZE = 9;

	for ( byte chunkIdx = 0; chunkIdx < numChunks; ++chunkIdx )
	{
		byte chunkType = bufferParser.ParseByte();
		uint32_t chunkOffset = bufferParser.ParseUint32();
		uint32_t chunkDataSize = bufferParser.ParseUint32();

		bufferParser.SetReadOffset( chunkOffset );

		ReadChunk( bufferParser, chunkType, chunkDataSize );

		bufferParser.SetEndianMode( tocEndianMode );
		bufferParser.SetReadOffset( offsetToTOC + (uint32_t)TOC_HEADER_SIZE + (uint32_t)( TOC_CHUNK_ENTRY_SIZE * ( chunkIdx + 1 ) ) );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::ReadChunk( BufferParser& bufferParser, byte expectedChunkType, uint32_t expectedChunkDataSize )
{
	if ( bufferParser.ParseChar() != '\0'
		 || bufferParser.ParseChar() != 'C'
		 || bufferParser.ParseChar() != 'H'
		 || bufferParser.ParseChar() != 'K' )
	{
		g_devConsole->PrintError( "Invalid Chunk: CC is not 0CHK" );
		return false;
	}

	// Chunk Type
	byte chunkType = bufferParser.ParseByte();
	if ( chunkType != expectedChunkType )
	{
		g_devConsole->PrintError( Stringf( "Invalid Chunk: Chunk type '%c' does not match expected type '%c'", chunkType, expectedChunkType ) );
		return false;
	}

	// Chunk Endianness
	byte endianMode = bufferParser.ParseByte();
	switch ( endianMode )
	{
		case 1: bufferParser.SetEndianMode( eBufferEndianMode::LITTLE ); break;
		case 2: bufferParser.SetEndianMode( eBufferEndianMode::BIG ); break;
		default:
		{
			g_devConsole->PrintError( "Invalid Chunk: Unsupported endian mode" );
			return false;
		}
	}

	// Chunk size
	uint32_t chunkDataSize = bufferParser.ParseUint32();
	if ( chunkDataSize != expectedChunkDataSize )
	{
		g_devConsole->PrintError( Stringf( "Invalid Chunk: Chunk data size '%i' does not match expected size '%i'", chunkDataSize, expectedChunkDataSize ) );
		return false;
	}

	switch ( chunkType )
	{
		case 1: return ReadConvexPolysChunk( bufferParser ); 
		case 128: return ReadSceneInfoChunk( bufferParser ); 
		case 129: return ReadBoundingDiscsChunk( bufferParser ); 
	}

	// Ignore unknown chunk
	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::ReadConvexPolysChunk( BufferParser& bufferParser )
{
	// Read in polys
	std::vector<Entity*> chunkEntities;
	uint32_t numPolys = bufferParser.ParseUint32(); 
	
	Entity::ResetEntityIds();
	
	for ( uint32_t polyIdx = 0; polyIdx < numPolys; ++polyIdx )
	{
		uint16_t numVerts = bufferParser.ParseUshort();
		if ( numVerts == (uint16_t)0 )
		{
			g_devConsole->PrintWarning( Stringf( "Convex object '%i' has no vertices defined", polyIdx ) );
			continue;
		}

		std::vector<Vec2> points;
		for ( int vertIdx = 0; vertIdx < numVerts; ++vertIdx )
		{
			points.push_back( bufferParser.ParseVec2() );
		}

		Polygon2 convexPolygon( points );
		Entity* newEntity = new Entity( convexPolygon );
		
		// Make a rough guess for center of polygon
		newEntity->SetPosition( convexPolygon.GetCenterOfMass() );

		chunkEntities.push_back( newEntity );
	}

	// Reload entities from chunk
	m_symmetricQuadTree->RemoveEntitiesAboveId( -1 );
	m_symmetricQuadTree->AddEntities( chunkEntities );

	PTR_VECTOR_SAFE_DELETE( m_entities );

	m_entities = chunkEntities;
	m_numActiveEntities = (int)m_entities.size();
	m_numTotalEntities = (int)m_entities.size();

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::ReadSceneInfoChunk( BufferParser& bufferParser )
{
	AABB2 sceneBounds = bufferParser.ParseAABB2();

	g_windowWidth = sceneBounds.GetWidth();
	g_windowHeight = sceneBounds.GetHeight();

	if ( !IsNearlyEqual( g_windowHeight, 0.f ) )
	{
		g_debugLineThickness = g_windowHeight / 300.f;
	}

	g_game->SetSceneCameras( sceneBounds );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ConvexRaycastMap::ReadBoundingDiscsChunk( BufferParser& bufferParser )
{
	uint32_t numEntities = bufferParser.ParseUint32();

	if ( numEntities != (uint32_t)m_numActiveEntities )
	{
		g_devConsole->PrintError( Stringf( "Bounding discs chunk defines %i objects, but %i objects were defined in the scene, ignoring bounding discs chunk", numEntities, m_numActiveEntities ) );
		return false;
	}

	for ( uint32_t polyIdx = 0; polyIdx < numEntities; ++polyIdx )
	{
		Entity*& entity = m_entities[polyIdx];

		entity->SetPosition( bufferParser.ParseVec2() );
		entity->SetPhysicsRadius( bufferParser.ParseFloat() );
	}

	return true;
}


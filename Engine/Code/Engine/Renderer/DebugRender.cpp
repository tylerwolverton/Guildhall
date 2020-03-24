#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------------------------
class DebugRenderObject;

static RenderContext* s_debugRenderContext = nullptr;
static Camera* s_debugCamera = nullptr;
static bool s_isDebugRenderEnabled = false;
static std::vector<DebugRenderObject*> s_debugRenderWorldObjects;
static std::vector<DebugRenderObject*> s_debugRenderScreenObjects;
static float s_screenHeight = 1080.f;


//-----------------------------------------------------------------------------------------------
// Private classes
//-----------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const Rgba8& startColor, const Rgba8& endColor, float duration = 0.f );
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<uint>& indices, const Rgba8& startColor, const Rgba8& endColor, float duration = 0.f );
	
	bool IsReadyToBeCulled() const;

	std::vector<Vertex_PCU> GetVertices() const { return m_vertices; }

public:
	std::vector<Vertex_PCU> m_vertices;
	std::vector<uint> m_indices;
	float m_duration = 0.f;
	Rgba8 m_startColor = Rgba8::MAGENTA;
	Rgba8 m_endColor = Rgba8::BLACK;
	Timer m_timer;
};


//-----------------------------------------------------------------------------------------------
DebugRenderObject::DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const Rgba8& startColor, const Rgba8& endColor, float duration )
	: m_vertices( vertices )
	, m_startColor( startColor )
	, m_endColor( endColor )
	, m_duration( duration )
{
	m_timer.m_clock = s_debugRenderContext->GetClock();
	if ( m_timer.m_clock == nullptr )
	{
		m_timer.m_clock = Clock::GetMaster();
	}

	m_timer.SetSeconds( m_duration );
	m_timer.Reset();
}


//-----------------------------------------------------------------------------------------------
DebugRenderObject::DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<uint>& indices, const Rgba8& startColor, const Rgba8& endColor, float duration )
	: m_vertices( vertices )
	, m_indices( indices )
	, m_startColor( startColor )
	, m_endColor( endColor )
	, m_duration( duration )
{
	m_timer.m_clock = s_debugRenderContext->GetClock();
	if ( m_timer.m_clock == nullptr )
	{
		m_timer.m_clock = Clock::GetMaster();
	}

	m_timer.SetSeconds( m_duration );
	m_timer.Reset();
}


//-----------------------------------------------------------------------------------------------
static void AppendDebugObjectToVertexArray( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, DebugRenderObject* obj )
{
	// Update color based on age
	std::vector<Vertex_PCU> objVerts = obj->GetVertices();
	for ( int vertIdx = 0; vertIdx < (int)objVerts.size(); ++vertIdx )
	{
		Vertex_PCU vertex = objVerts[vertIdx];

		vertex.m_color = InterpolateColor( obj->m_startColor, obj->m_endColor, (float)obj->m_timer.GetElapsedSeconds() / obj->m_duration );

		vertices.push_back( vertex );
	}

	indices.insert( indices.end(), obj->m_indices.begin(), obj->m_indices.end() );
}


//-----------------------------------------------------------------------------------------------
bool DebugRenderObject::IsReadyToBeCulled() const
{
	return m_timer.HasElapsed();
}


//-----------------------------------------------------------------------------------------------
// Debug Render System
//-----------------------------------------------------------------------------------------------
void DebugRenderSystemStartup( RenderContext* context )
{
	s_debugRenderContext = context;
	s_debugCamera = new Camera();
}


//-----------------------------------------------------------------------------------------------
void DebugRenderSystemShutdown()
{
	PTR_SAFE_DELETE( s_debugCamera );
	ClearDebugRendering();
}


//-----------------------------------------------------------------------------------------------
void EnableDebugRendering()
{
	s_isDebugRenderEnabled = true;
}


//-----------------------------------------------------------------------------------------------
void DisableDebugRendering()
{
	s_isDebugRenderEnabled = false;
}


//-----------------------------------------------------------------------------------------------
void ClearDebugRendering()
{
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldObjects );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderScreenObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
void DebugRenderWorldToCamera( Camera* camera )
{
	if ( !s_isDebugRenderEnabled )
	{
		return;
	}

	s_debugCamera->SetClearMode( CLEAR_NONE );
	s_debugCamera->SetTransform( camera->GetTransform() );
	s_debugCamera->SetColorTarget( camera->GetColorTarget() );
	s_debugCamera->SetOutputSize( camera->GetOutputSize() );
	s_debugCamera->SetDepthStencilTarget( camera->GetDepthStencilTarget() );
	s_debugCamera->SetViewMatrix( camera->GetViewMatrix() );
	s_debugCamera->SetProjectionMatrix( camera->GetProjectionMatrix() );
	
	s_debugRenderContext->BeginCamera( *s_debugCamera );

	for ( int debugObjIdx = 0; debugObjIdx < (int)s_debugRenderWorldObjects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = s_debugRenderWorldObjects[debugObjIdx];
		if ( obj != nullptr )
		{
			std::vector<Vertex_PCU> vertices;
			std::vector<uint> indices;

			AppendDebugObjectToVertexArray( vertices, indices, obj );

			GPUMesh mesh( s_debugRenderContext, vertices, indices );
			s_debugRenderContext->DrawMesh( &mesh );
		}
	}
	
	//if ( vertices.size() > 0 )
	//{
	//	s_debugRenderContext->DrawVertexArray( vertices );
	//}

	s_debugRenderContext->EndCamera( *s_debugCamera );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderScreenTo( Texture* output )
{
	if ( !s_isDebugRenderEnabled )
	{
		return;
	}

	RenderContext* context = output->m_owner;

	s_debugCamera->SetColorTarget( output );
	
	s_debugCamera->SetOutputSize( Vec2( 1920.f, s_screenHeight ) );
	IntVec2 max = output->GetTexelSize();
	s_debugCamera->SetProjectionOrthographic( s_screenHeight, -1.f, 1.f );
	s_debugCamera->SetPosition( Vec3( 1920.f, 1080.f, 0.f ) * .5f );

	s_debugCamera->SetClearMode( CLEAR_NONE );

	context->BeginCamera( *s_debugCamera );

	//context->SetDepthTest( eCompareFunc::COMPARISON_ALWAYS, true );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	for ( int debugObjIdx = 0; debugObjIdx < (int)s_debugRenderScreenObjects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = s_debugRenderScreenObjects[debugObjIdx];
		if ( obj != nullptr )
		{
			AppendDebugObjectToVertexArray( vertices, indices, obj );
		}
	}

	if ( vertices.size() > 0 )
	{
		context->DrawVertexArray( vertices );
	}

	context->EndCamera( *s_debugCamera );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{
	for ( int debugObjIdx = 0; debugObjIdx < (int)s_debugRenderWorldObjects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = s_debugRenderWorldObjects[debugObjIdx];
		if ( obj != nullptr
			 && obj->IsReadyToBeCulled() )
		{
			PTR_SAFE_DELETE( obj );
		}
	}

	for ( int debugObjIdx = 0; debugObjIdx < (int)s_debugRenderScreenObjects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = s_debugRenderScreenObjects[debugObjIdx];
		if ( obj != nullptr
			 && obj->IsReadyToBeCulled() )
		{
			PTR_SAFE_DELETE( obj );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AABB3 pointBounds( Vec3::ZERO, Vec3( size, size, size ) );
	pointBounds.SetCenter( pos );

	AppendVertsAndIndicesForSphereMesh( vertices, indices, pos, size, 16, 16, start_color );
	
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );
	
	// Update to find next open slot?
	s_debugRenderWorldObjects.push_back( obj );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, size, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, .01f, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldLine( const Vec3& p0, const Rgba8& p0_start_color, const Rgba8& p0_end_color, 
						const Vec3& p1, const Rgba8& p1_start_color, const Rgba8& p1_end_color, 
						float duration, eDebugRenderMode mode )
{
	//UNUSED( p1_start_color );
	UNUSED( p1_end_color );
	UNUSED( mode );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec3 obbBone = p1 - p0; // k vector of obb3
	Vec3 normalizedK = obbBone.GetNormalized();
	Vec3 iBasis = CrossProduct3D( Vec3( 0.f, 1.f, 0.f ), normalizedK );
	Vec3 jBasis = CrossProduct3D( normalizedK, iBasis );

	Vec3 obbCenter = p0 + ( obbBone * .5f );
	Vec3 obbDimensions( .01f, .01f, obbBone.GetLength() );

	OBB3 lineBounds( obbCenter, obbDimensions, iBasis, jBasis );

	AppendVertsForOBB3D( vertices, lineBounds, p0_start_color );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, p0_start_color, p0_end_color, duration );

	s_debugRenderWorldObjects.push_back( obj );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldLine( const Vec3& start, const Vec3& end, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldLine( start, color, color, end, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderSetScreenHeight( float height )
{
	s_screenHeight = height;
}


//-----------------------------------------------------------------------------------------------
AABB2 DebugGetScreenBounds()
{
	return AABB2::ONE_BY_ONE;
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 start_color, Rgba8 end_color, float duration )
{
	std::vector<Vertex_PCU> vertices;

	AABB2 pointBounds( Vec2::ZERO, Vec2( size, size ) );
	pointBounds.SetCenter( pos );

	AppendVertsForArc( vertices, pos, size, 360.f, 0.f, start_color );
	
	DebugRenderObject* obj = new DebugRenderObject( vertices, start_color, end_color, duration );

	// Update to find next open slot?
	s_debugRenderScreenObjects.push_back( obj );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, float size, Rgba8 color, float duration )
{
	DebugAddScreenPoint( pos, size, color, color, duration );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, Rgba8 color )
{
	DebugAddScreenPoint( pos, 1.f, color, 0.f );
}

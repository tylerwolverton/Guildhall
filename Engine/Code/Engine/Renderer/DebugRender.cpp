#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
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
static bool s_isDebugRenderEnabled = false;
static std::vector<DebugRenderObject*> s_debugRenderObjects;


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

		vertex.m_color = InterpolateColor( obj->m_startColor, obj->m_endColor, obj->m_timer.GetElapsedSeconds() / obj->m_duration );

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
}


//-----------------------------------------------------------------------------------------------
void DebugRenderSystemShutdown()
{
	PTR_VECTOR_SAFE_DELETE( s_debugRenderObjects );
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
	PTR_VECTOR_SAFE_DELETE( s_debugRenderObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
void DebugRenderWorldToCamera( Camera* camera )
{
	camera->SetClearMode( CLEAR_NONE );

	

	s_debugRenderContext->BeginCamera( *camera );

	for ( int debugObjIdx = 0; debugObjIdx < (int)s_debugRenderObjects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = s_debugRenderObjects[debugObjIdx];
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

	s_debugRenderContext->EndCamera( *camera );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderScreenTo( Texture* output )
{

}


//-----------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{
	for ( int debugObjIdx = 0; debugObjIdx < (int)s_debugRenderObjects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = s_debugRenderObjects[debugObjIdx];
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
	if ( !s_isDebugRenderEnabled )
	{
		return;
	}

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AABB3 pointBounds( Vec3::ZERO, Vec3( size, size, size ) );
	pointBounds.SetCenter( pos );

	AppendVertsForAABB3D( vertices, pointBounds, start_color );
	
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );
	
	// Update to find next open slot?
	s_debugRenderObjects.push_back( obj );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& color, float duration, eDebugRenderMode mode )
{

}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, const Rgba8& color, float duration, eDebugRenderMode mode )
{

}

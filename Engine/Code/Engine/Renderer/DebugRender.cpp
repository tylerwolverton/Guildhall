#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"

//-----------------------------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------------------------
class DebugRenderObject;

static Clock* s_clock = nullptr;
static RenderContext* s_debugRenderContext = nullptr;
static Camera* s_debugCamera = nullptr;
static bool s_isDebugRenderEnabled = false;
static std::vector<DebugRenderObject> s_debugRenderObjects;


//-----------------------------------------------------------------------------------------------
// Private classes
//-----------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const Rgba8& startColor, const Rgba8& endColor, float duration = 0.f );
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<int>& indices, const Rgba8& startColor, const Rgba8& endColor, float duration = 0.f );
	bool IsReadyToBeCulled() const;

private:
	std::vector<Vertex_PCU> m_vertices;
	std::vector<int> m_indices;
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
	m_timer.m_clock = s_clock;
}


//-----------------------------------------------------------------------------------------------
DebugRenderObject::DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<int>& indices, const Rgba8& startColor, const Rgba8& endColor, float duration )
	: m_vertices( vertices )
	, m_indices( indices )
	, m_startColor( startColor )
	, m_endColor( endColor )
	, m_duration( duration )
{
	m_timer.m_clock = s_clock;
}


//-----------------------------------------------------------------------------------------------
bool DebugRenderObject::IsReadyToBeCulled() const
{
	return m_timer.HasElapsed();
}


//-----------------------------------------------------------------------------------------------
// Debug Render System
//-----------------------------------------------------------------------------------------------
void DebugRenderSystemStartup( Clock* clock )
{
	s_clock = clock;
}


//-----------------------------------------------------------------------------------------------
void DebugRenderSystemShutdown()
{

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
	s_debugRenderObjects.clear();
}


//-----------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{

}


//-----------------------------------------------------------------------------------------------
void DebugRenderWorldToCamera( Camera* cam )
{

}


//-----------------------------------------------------------------------------------------------
void DebugRenderScreenTo( Texture* output )
{

}


//-----------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{

}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode )
{

}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& color, float duration, eDebugRenderMode mode )
{

}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, const Rgba8& color, float duration, eDebugRenderMode mode )
{

}

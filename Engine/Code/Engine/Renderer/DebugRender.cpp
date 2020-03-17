#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
// Private classes
//-----------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	bool IsReadyToBeCulled() const;

private:
	std::vector<Vertex_PCU> m_vertices;
	std::vector<int> m_indices;
	float m_age = 0.f;
	float m_lifespan = 0.f;

};


//-----------------------------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------------------------
static RenderContext* s_debugRenderContext = nullptr;
static Camera* s_debugCamera = nullptr;
static bool s_isDebugRenderEnabled = false;
static std::vector<DebugRenderObject> s_debugRenderObjects;


//-----------------------------------------------------------------------------------------------
void DebugRenderSystemStartup()
{

}


//-----------------------------------------------------------------------------------------------
void DebugRenderSystemShutdown()
{
	if ( s_debugCamera != nullptr )
	{
		
	}
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

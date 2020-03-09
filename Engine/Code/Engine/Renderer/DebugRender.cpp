#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Texture.hpp"

// Static variables here?
static RenderContext* s_renderer = nullptr;

// Or internal class?
class DebugRenderSystem
{
	RenderContext* m_context;
	Camera* m_camera;
};

static DebugRenderSystem* debugRenderSystem = nullptr;

class DebugRenderObject
{
public:
	bool isReadyToBeCulled() const;
};


//-----------------------------------------------------------------------------------------------
void DebugRenderScreenTo( Texture* output )
{
	RenderContext* context = output->m_owner;
	s_renderer = context;

	TextureView* rtv = output->GetOrCreateRenderTargetView();

	Camera camera;
	camera.SetColorTarget( output );
	Vec2 min = Vec2::ZERO;
	Vec2 max = output->GetTexelSize();
	camera.SetProjectionOrthographic( min, max );

	camera.SetClearMode( CLEAR_NONE );

	std::vector<Vertex_PCU> vertices;

	context->BeginCamera( camera );

	foreach( obj : objList)
	{
		AppendObjectToVertexArray( vertices, indices, obj );
	}

	context->DrawVertexArray( vertices );

	context->EndCamera( camera );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{
	// remove and delete each object that is ready to be culled
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenPoint( Vec2 pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration )
{
	// Make an object
	SomeObject* obj = CreateObjectForThisScreenPoint( pos, size, start_color, end_color, duration );
	AppendObjectToList( obj );
}

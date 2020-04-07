#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------------------------
class DebugRenderObject;

static RenderContext* s_debugRenderContext = nullptr;
static Camera* s_debugCamera = nullptr;
static bool s_isDebugRenderEnabled = false;

// Depth
static std::vector<DebugRenderObject*> s_debugRenderWorldObjects;
static std::vector<DebugRenderObject*> s_debugRenderWorldOutlineObjects;
static std::vector<DebugRenderObject*> s_debugRenderWorldTextObjects;
static std::vector<DebugRenderObject*> s_debugRenderWorldBillboardTextObjects;

// Always
static std::vector<DebugRenderObject*> s_debugRenderWorldObjectsAlways;
static std::vector<DebugRenderObject*> s_debugRenderWorldOutlineObjectsAlways;
static std::vector<DebugRenderObject*> s_debugRenderWorldTextObjectsAlways;
static std::vector<DebugRenderObject*> s_debugRenderWorldBillboardTextObjectsAlways;

// X-ray
static std::vector<DebugRenderObject*> s_debugRenderWorldObjectsXRay;
static std::vector<DebugRenderObject*> s_debugRenderWorldOutlineObjectsXRay;
static std::vector<DebugRenderObject*> s_debugRenderWorldTextObjectsXRay;
static std::vector<DebugRenderObject*> s_debugRenderWorldBillboardTextObjectsXRay;

// Screen 
static std::vector<DebugRenderObject*> s_debugRenderScreenObjects;
static std::vector<DebugRenderObject*> s_debugRenderScreenTexturedObjects;
static std::vector<DebugRenderObject*> s_debugRenderScreenTextObjects;

static float s_screenWidth = 1920.f;
static float s_screenHeight = 1080.f;


//-----------------------------------------------------------------------------------------------
// Data Helpers
//-----------------------------------------------------------------------------------------------
static void AddDebugRenderObjectToVector( DebugRenderObject* newObject, std::vector<DebugRenderObject*>& objectVector )
{
	for ( int i = 0; i < (int)objectVector.size(); ++i )
	{
		if ( objectVector[i] == nullptr )
		{
			objectVector[i] = newObject;
			return;
		}
	}

	objectVector.push_back( newObject );
}


//-----------------------------------------------------------------------------------------------
// Private classes
//-----------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const Rgba8& startColor, const Rgba8& endColor, float duration = 0.f );
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const Rgba8& startColor, const Rgba8& endColor, Texture* texture, float duration = 0.f  );
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<uint>& indices, const Rgba8& startColor, const Rgba8& endColor, float duration = 0.f );
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<uint>& indices, const Rgba8& startColor, const Rgba8& endColor, const Vec3& origin, float duration = 0.f  );
	
	bool IsReadyToBeCulled() const;

	std::vector<Vertex_PCU> GetVertices() const { return m_vertices; }

public:
	std::vector<Vertex_PCU> m_vertices;
	std::vector<uint> m_indices;
	float m_duration = 0.f;
	Vec3 m_origin = Vec3::ZERO;
	Texture* m_texture = nullptr;
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
DebugRenderObject::DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<uint>& indices, const Rgba8& startColor, const Rgba8& endColor, const Vec3& origin, float duration )
	: m_vertices( vertices )
	, m_indices( indices )
	, m_startColor( startColor )
	, m_endColor( endColor )
	, m_duration( duration )
	, m_origin( origin )
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
DebugRenderObject::DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const Rgba8& startColor, const Rgba8& endColor, Texture* texture, float duration )
	: m_vertices( vertices )
	, m_startColor( startColor )
	, m_endColor( endColor )
	, m_duration( duration )
	, m_texture( texture )
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
static void AppendDebugObjectToVertexArrayAndLerpTint( std::vector<Vertex_PCU>& vertices, std::vector<uint>& indices, DebugRenderObject* obj )
{
	// Update color based on age
	std::vector<Vertex_PCU> objVerts = obj->GetVertices();
	for ( int vertIdx = 0; vertIdx < (int)objVerts.size(); ++vertIdx )
	{
		Vertex_PCU vertex = objVerts[vertIdx];

		vertex.m_color = InterpolateColor( obj->m_startColor, obj->m_endColor, obj->m_timer.GetRatioOfCompletion() );

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
// Dev console event handlers
//-----------------------------------------------------------------------------------------------
static bool DebugRenderEnableEvent( EventArgs* args )
{
	if ( args->GetValue( "enabled", true ) )
	{
		EnableDebugRendering();
	}
	else
	{
		DisableDebugRendering();
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
static bool DebugRenderWorldPointEvent( EventArgs* args )
{
	Vec3 pos = args->GetValue( "position", Vec3::ZERO );
	float duration = args->GetValue( "duration", 5.f );
	
	DebugAddWorldPoint( pos, Rgba8::GREEN, duration, DEBUG_RENDER_XRAY );

	return false;
}


//-----------------------------------------------------------------------------------------------
static bool DebugRenderWorldWireSphereEvent( EventArgs* args )
{
	Vec3 pos = args->GetValue( "position", Vec3::ZERO );
	float radius = args->GetValue( "radius", 1.f );
	float duration = args->GetValue( "duration", 5.f );
	
	DebugAddWorldWireSphere( pos, radius, Rgba8::GREEN, duration, DEBUG_RENDER_XRAY );

	return false;
}


//-----------------------------------------------------------------------------------------------
static bool DebugRenderWorldWireBoundsEvent( EventArgs* args )
{
	Vec3 min = args->GetValue( "min", Vec3::ZERO );
	Vec3 max = args->GetValue( "max", Vec3::ONE );
	float duration = args->GetValue( "duration", 5.f );

	DebugAddWorldWireBounds( AABB3( min, max ), Rgba8::GREEN, duration, DEBUG_RENDER_XRAY );

	return false;
}


//-----------------------------------------------------------------------------------------------
static bool DebugRenderWorldBillboardTextEvent( EventArgs* args )
{
	Vec3 pos = args->GetValue( "position", Vec3::ZERO );
	Vec2 pivot = args->GetValue( "pivot", Vec2( .5f, .5f ) );
	std::string text = args->GetValue( "text", "Hello" );
	float duration = args->GetValue( "duration", 5.f );

	DebugAddWorldBillboardText( pos, pivot, Rgba8::RED, Rgba8::RED, duration, DEBUG_RENDER_XRAY, text.c_str() );

	return false;
}


//-----------------------------------------------------------------------------------------------
static bool DebugRenderScreenPointEvent( EventArgs* args )
{
	Vec2 pos = args->GetValue( "position", Vec2::ZERO );
	float duration = args->GetValue( "duration", 5.f );

	DebugAddScreenPoint( pos, 10.f, Rgba8::GREEN, duration );

	return false;
}


//-----------------------------------------------------------------------------------------------
static bool DebugRenderScreenQuadEvent( EventArgs* args )
{
	Vec2 min = args->GetValue( "min", Vec2::ZERO );
	Vec2 max = args->GetValue( "max", Vec2::ONE );
	float duration = args->GetValue( "duration", 5.f );

	DebugAddScreenQuad( AABB2( min, max ), Rgba8::CYAN, duration );

	return false;
}


//-----------------------------------------------------------------------------------------------
static bool DebugRenderScreenTextEvent( EventArgs* args )
{
	if ( args->GetValue( "enabled", false ) )
	{
		EnableDebugRendering();
	}
	else
	{
		DisableDebugRendering();
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Debug Render System
//-----------------------------------------------------------------------------------------------
void DebugRenderSystemStartup( RenderContext* context, EventSystem* eventSystem )
{
	s_debugRenderContext = context;
	s_debugCamera = new Camera();

	eventSystem->RegisterEvent( "debug_render", "Usage: debug_render enabled=bool", eUsageLocation::DEV_CONSOLE, DebugRenderEnableEvent );
	eventSystem->RegisterEvent( "debug_add_world_point", "Usage: debug_add_world_point position=vec3 duration=float", eUsageLocation::DEV_CONSOLE, DebugRenderWorldPointEvent );
	eventSystem->RegisterEvent( "debug_add_world_wire_sphere", "Usage: debug_add_world_wire_sphere position=vec3 radius=float duration=float", eUsageLocation::DEV_CONSOLE, DebugRenderWorldWireSphereEvent );
	eventSystem->RegisterEvent( "debug_add_world_wire_bounds", "Usage: debug_add_world_wire_bounds min=vec3 max=vec3 duration=float", eUsageLocation::DEV_CONSOLE, DebugRenderWorldWireBoundsEvent );
	eventSystem->RegisterEvent( "debug_add_world_billboard_text", "Usage: debug_add_world_billboard_text position=vec3 pivot=vec2 text=string duration=float", eUsageLocation::DEV_CONSOLE, DebugRenderWorldBillboardTextEvent );
	eventSystem->RegisterEvent( "debug_add_screen_point", "Usage: debug_add_screen_point position=vec2 duration=float", eUsageLocation::DEV_CONSOLE, DebugRenderScreenPointEvent );
	eventSystem->RegisterEvent( "debug_add_screen_quad", "Usage: debug_add_screen_quad min=vec2 max=vec2 duration=float", eUsageLocation::DEV_CONSOLE, DebugRenderScreenQuadEvent );
	eventSystem->RegisterEvent( "debug_add_screen_text", "Usage: debug_add_screen_text position=vec2 pivot=vec2 text=string", eUsageLocation::DEV_CONSOLE, DebugRenderScreenTextEvent );
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
	// Depth
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldObjects );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldOutlineObjects );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldTextObjects );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldBillboardTextObjects );
	// Always
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldObjectsAlways );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldOutlineObjectsAlways );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldTextObjectsAlways );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldBillboardTextObjectsAlways );
	// XRay
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldObjectsXRay );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldOutlineObjectsXRay );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldTextObjectsXRay );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderWorldBillboardTextObjectsXRay );
	// Screen
	PTR_VECTOR_SAFE_DELETE( s_debugRenderScreenObjects );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderScreenTexturedObjects );
	PTR_VECTOR_SAFE_DELETE( s_debugRenderScreenTextObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
static void InitializeDebugCamera( Camera* camera )
{
	s_debugCamera->SetTransform( camera->GetTransform() );
	s_debugCamera->SetColorTarget( camera->GetColorTarget() );
	s_debugCamera->SetOutputSize( camera->GetOutputSize() );
	s_debugCamera->SetDepthStencilTarget( camera->GetDepthStencilTarget() );
	s_debugCamera->SetViewMatrix( camera->GetViewMatrix() );
	s_debugCamera->SetProjectionMatrix( camera->GetProjectionMatrix() );

	s_debugCamera->SetClearMode( CLEAR_NONE );
}


//-----------------------------------------------------------------------------------------------
static void RenderWorldObjects( const std::vector<DebugRenderObject*> objects )
{
	for ( int debugObjIdx = 0; debugObjIdx < (int)objects.size(); ++debugObjIdx )
	{
		DebugRenderObject* const& obj = objects[debugObjIdx];
		if ( obj != nullptr )
		{			
			s_debugRenderContext->SetModelMatrix( Mat44(), obj->m_startColor );
			s_debugRenderContext->SetMaterialData( obj->m_startColor, obj->m_endColor, obj->m_timer.GetRatioOfCompletion() );

			GPUMesh mesh( s_debugRenderContext, obj->m_vertices, obj->m_indices );
			s_debugRenderContext->DrawMesh( &mesh );
		}
	}

	s_debugRenderContext->SetModelMatrix( Mat44() );
}


//-----------------------------------------------------------------------------------------------
static void RenderWorldBillboardTextObjects( const std::vector<DebugRenderObject*> objects )
{
	for ( int debugObjIdx = 0; debugObjIdx < (int)objects.size(); ++debugObjIdx )
	{
		DebugRenderObject*const& obj = objects[debugObjIdx];
		if ( obj != nullptr )
		{			
			Mat44 model = s_debugCamera->GetTransform().GetAsMatrix();
			model.SetTranslation3D( obj->m_origin );
			model.PushTransform( Mat44::CreateTranslation3D( -obj->m_origin ) );
			
			s_debugRenderContext->SetModelMatrix( Mat44(), obj->m_startColor );
			s_debugRenderContext->SetMaterialData( obj->m_startColor, obj->m_endColor, obj->m_timer.GetRatioOfCompletion() );

			GPUMesh mesh( s_debugRenderContext, obj->m_vertices, obj->m_indices );
			s_debugRenderContext->DrawMesh( &mesh );
		}
	}

	s_debugRenderContext->SetModelMatrix( Mat44() );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderWorldToCamera( Camera* camera )
{
	if ( !s_isDebugRenderEnabled )
	{
		return;
	}

	InitializeDebugCamera( camera );

	s_debugRenderContext->BeginCamera( *s_debugCamera );

	s_debugRenderContext->BindShader( "Data/Shaders/DebugRender.hlsl" ); 
	BitmapFont* font = s_debugRenderContext->GetSystemFont();

	// Draw Depth
	s_debugRenderContext->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );
	s_debugRenderContext->BindDiffuseTexture( nullptr );
	RenderWorldObjects( s_debugRenderWorldObjects );
	RenderWorldObjects( s_debugRenderWorldObjectsXRay );
	
	s_debugRenderContext->SetFillMode( eFillMode::WIREFRAME );
	s_debugRenderContext->SetCullMode( eCullMode::NONE );
	RenderWorldObjects( s_debugRenderWorldOutlineObjects );
	RenderWorldObjects( s_debugRenderWorldOutlineObjectsXRay );
	s_debugRenderContext->SetFillMode( eFillMode::SOLID );
	s_debugRenderContext->SetCullMode( eCullMode::BACK );
	
	s_debugRenderContext->BindDiffuseTexture( font->GetTexture() );
	RenderWorldObjects( s_debugRenderWorldTextObjects );
	RenderWorldObjects( s_debugRenderWorldTextObjectsXRay );
	RenderWorldBillboardTextObjects( s_debugRenderWorldBillboardTextObjects );
	RenderWorldBillboardTextObjects( s_debugRenderWorldBillboardTextObjectsXRay );

	// Draw Always
	s_debugRenderContext->SetDepthTest( eCompareFunc::COMPARISON_ALWAYS, false );

	s_debugRenderContext->BindDiffuseTexture( nullptr );
	RenderWorldObjects( s_debugRenderWorldObjectsAlways );

	s_debugRenderContext->SetFillMode( eFillMode::WIREFRAME );
	s_debugRenderContext->SetCullMode( eCullMode::NONE );
	RenderWorldObjects( s_debugRenderWorldOutlineObjectsAlways );
	s_debugRenderContext->SetFillMode( eFillMode::SOLID );
	s_debugRenderContext->SetCullMode( eCullMode::BACK );

	s_debugRenderContext->BindDiffuseTexture( font->GetTexture() );
	RenderWorldObjects( s_debugRenderWorldTextObjectsAlways );
	RenderWorldBillboardTextObjects( s_debugRenderWorldBillboardTextObjectsAlways );

	// Draw XRay
	s_debugRenderContext->SetDepthTest( eCompareFunc::COMPARISON_GREATER, false );
	s_debugRenderContext->BindShader( "Data/Shaders/DebugRenderXRay.hlsl" );

	s_debugRenderContext->BindDiffuseTexture( nullptr );
	RenderWorldObjects( s_debugRenderWorldObjectsXRay );

	s_debugRenderContext->SetFillMode( eFillMode::WIREFRAME );
	s_debugRenderContext->SetCullMode( eCullMode::NONE );
	RenderWorldObjects( s_debugRenderWorldOutlineObjectsXRay );
	s_debugRenderContext->SetFillMode( eFillMode::SOLID );
	s_debugRenderContext->SetCullMode( eCullMode::BACK );

	s_debugRenderContext->BindDiffuseTexture( font->GetTexture() );
	RenderWorldObjects( s_debugRenderWorldTextObjectsXRay );
	RenderWorldBillboardTextObjects( s_debugRenderWorldBillboardTextObjectsXRay );

	s_debugRenderContext->EndCamera( *s_debugCamera );
}


//-----------------------------------------------------------------------------------------------
static void RenderScreenObjects( RenderContext* context, std::vector<DebugRenderObject*> objects )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	for ( int debugObjIdx = 0; debugObjIdx < (int)objects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = objects[debugObjIdx];
		if ( obj != nullptr )
		{
			AppendDebugObjectToVertexArrayAndLerpTint( vertices, indices, obj );
		}
	}

	if ( vertices.size() > 0 )
	{
		context->DrawVertexArray( vertices );
	}
}


//-----------------------------------------------------------------------------------------------
static void RenderTexturedScreenObjects( RenderContext* context, std::vector<DebugRenderObject*> objects )
{
	for ( int debugObjIdx = 0; debugObjIdx < (int)objects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = objects[debugObjIdx];
		if ( obj != nullptr )
		{
			std::vector<Vertex_PCU> vertices;
			std::vector<uint> indices;

			context->BindDiffuseTexture( obj->m_texture );
			AppendDebugObjectToVertexArrayAndLerpTint( vertices, indices, obj );
			context->DrawVertexArray( vertices );
		}
	}
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
	
	float aspect = output->GetAspectRatio();
	s_screenWidth = s_screenHeight * aspect;
	s_debugCamera->SetOutputSize( Vec2( s_screenWidth, s_screenHeight ) );
	
	s_debugCamera->SetTransform(  Transform() );
	s_debugCamera->SetPosition( Vec3( s_debugCamera->GetOutputSize(), 0.f ) * .5f );
	s_debugCamera->SetProjectionOrthographic( s_screenHeight );

	s_debugCamera->SetDepthStencilTarget( nullptr );
	s_debugCamera->SetClearMode( CLEAR_NONE );

	context->BeginCamera( *s_debugCamera );

	context->BindShader( "Data/Shaders/Default.hlsl" );

	context->SetCullMode( eCullMode::NONE );
	context->SetBlendMode( eBlendMode::ALPHA );
		
	RenderScreenObjects( context, s_debugRenderScreenObjects );
	RenderTexturedScreenObjects( context, s_debugRenderScreenTexturedObjects );

	BitmapFont* font = s_debugRenderContext->GetSystemFont();
	context->BindDiffuseTexture( font->GetTexture() );
	RenderScreenObjects( context, s_debugRenderScreenTextObjects );
	context->BindDiffuseTexture( nullptr );

	context->EndCamera( *s_debugCamera );
}


//-----------------------------------------------------------------------------------------------
static void CullExpiredObjects( std::vector<DebugRenderObject*>& objects )
{
	for ( int debugObjIdx = 0; debugObjIdx < (int)objects.size(); ++debugObjIdx )
	{
		DebugRenderObject*& obj = objects[debugObjIdx];
		if ( obj != nullptr
			 && obj->IsReadyToBeCulled() )
		{
			PTR_SAFE_DELETE( obj );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void DebugRenderEndFrame()
{
	// Depth
	CullExpiredObjects( s_debugRenderWorldObjects );
	CullExpiredObjects( s_debugRenderWorldOutlineObjects );
	CullExpiredObjects( s_debugRenderWorldTextObjects );
	CullExpiredObjects( s_debugRenderWorldBillboardTextObjects );
	// Always
	CullExpiredObjects( s_debugRenderWorldObjectsAlways );
	CullExpiredObjects( s_debugRenderWorldOutlineObjectsAlways );
	CullExpiredObjects( s_debugRenderWorldTextObjectsAlways );
	CullExpiredObjects( s_debugRenderWorldBillboardTextObjectsAlways );
	// XRay
	CullExpiredObjects( s_debugRenderWorldObjectsXRay );
	CullExpiredObjects( s_debugRenderWorldOutlineObjectsXRay );
	CullExpiredObjects( s_debugRenderWorldTextObjectsXRay );
	CullExpiredObjects( s_debugRenderWorldBillboardTextObjectsXRay );
	// Screen
	CullExpiredObjects( s_debugRenderScreenObjects );
	CullExpiredObjects( s_debugRenderScreenTexturedObjects );
	CullExpiredObjects( s_debugRenderScreenTextObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AABB3 pointBounds( Vec3::ZERO, Vec3( size, size, size ) );
	pointBounds.SetCenter( pos );
		
	AppendVertsForCubeMesh( vertices, pos, size, Rgba8::WHITE );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );
	
	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: AddDebugRenderObjectToVector(obj, s_debugRenderWorldObjects ); return;
		case DEBUG_RENDER_ALWAYS: AddDebugRenderObjectToVector( obj, s_debugRenderWorldObjectsAlways ); return;
		case DEBUG_RENDER_XRAY: AddDebugRenderObjectToVector( obj, s_debugRenderWorldObjectsXRay ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, size, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( pos, .05f, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec2& pos, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldPoint( Vec3( pos, 0.f ), color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldLine( const Vec3& p0, const Rgba8& p0_color, const Rgba8& p1_color, 
						const Vec3& p1, const Rgba8& start_tint, const Rgba8& end_tint, 
						float duration, eDebugRenderMode mode )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec3 obbBone = p1 - p0; // k vector of obb3
	Vec3 normalizedK = obbBone.GetNormalized();

	Mat44 lookAt = MakeLookAtMatrix( p0, p1 );

	Vec3 obbCenter = p0 + ( obbBone * .5f );
	Vec3 obbDimensions( .01f, .01f, obbBone.GetLength() );

	OBB3 lineBounds( obbCenter, obbDimensions, lookAt.GetIBasis3D(), lookAt.GetJBasis3D() );

	AppendVertsForOBB3D( vertices, lineBounds, p0_color, p1_color );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_tint, end_tint, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: AddDebugRenderObjectToVector( obj, s_debugRenderWorldObjects ); return;
		case DEBUG_RENDER_ALWAYS: AddDebugRenderObjectToVector( obj, s_debugRenderWorldObjectsAlways ); return;
		case DEBUG_RENDER_XRAY: AddDebugRenderObjectToVector( obj, s_debugRenderWorldObjectsXRay ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldLine( const Vec3& start, const Vec3& end, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldLine( start, color, color, end, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldArrow( const Vec3& p0, const Rgba8& p0_color, const Rgba8& p1_color, 
						 const Vec3& p1, const Rgba8& start_tint, const Rgba8& end_tint, 
						 float duration, eDebugRenderMode mode )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec3 bone( p1 - p0 );
	float cylinderLength = bone.GetLength() * .9f;

	Mat44 lookAt = MakeLookAtMatrix( p0, p1 );

	Vec3 endOfLine = p0 + lookAt.GetKBasis3D() * cylinderLength;

	AppendVertsAndIndicesForCylinderMesh( vertices, indices, p0, endOfLine, .03f, .03f, p0_color, p1_color );
	DebugRenderObject* cylinderObj = new DebugRenderObject( vertices, indices, start_tint, end_tint, duration );

	vertices.clear();
	indices.clear();
	AppendVertsAndIndicesForConeMesh( vertices, indices, endOfLine, p1, .15f, p1_color );
	DebugRenderObject* coneObj = new DebugRenderObject( vertices, indices, start_tint, end_tint, duration );
	

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: AddDebugRenderObjectToVector( cylinderObj, s_debugRenderWorldObjects ); AddDebugRenderObjectToVector( coneObj, s_debugRenderWorldObjects ); return;
		case DEBUG_RENDER_ALWAYS: AddDebugRenderObjectToVector( cylinderObj, s_debugRenderWorldObjectsAlways );  AddDebugRenderObjectToVector( coneObj, s_debugRenderWorldObjectsAlways ); return;
		case DEBUG_RENDER_XRAY: AddDebugRenderObjectToVector( cylinderObj, s_debugRenderWorldObjectsXRay ); AddDebugRenderObjectToVector( coneObj, s_debugRenderWorldObjectsXRay ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldArrow( const Vec3& start, const Vec3& end, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldArrow( start, color, color, end, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldArrow( const Vec2& start, const Vec2& end, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldArrow( Vec3( start, 0.f ), Vec3( end, 0.f ), color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldQuad( const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const AABB2& uvs, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec3 corners[] = {
		p0,p1,p2,p3,
		p0,p1,p2,p3
	};

	AppendVertsFor3DBox( vertices, 8, corners, Rgba8::WHITE, Rgba8::WHITE, uvs.mins, uvs.maxs );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: AddDebugRenderObjectToVector( obj, s_debugRenderWorldObjects ); return;
		case DEBUG_RENDER_ALWAYS: AddDebugRenderObjectToVector( obj, s_debugRenderWorldObjectsAlways ); return;
		case DEBUG_RENDER_XRAY: AddDebugRenderObjectToVector( obj, s_debugRenderWorldObjectsXRay ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldWireBounds( const OBB3& bounds, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendVertsForOBB3D( vertices, bounds, Rgba8::WHITE );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: AddDebugRenderObjectToVector( obj, s_debugRenderWorldOutlineObjects ); return;
		case DEBUG_RENDER_ALWAYS: AddDebugRenderObjectToVector( obj, s_debugRenderWorldOutlineObjectsAlways ); return;
		case DEBUG_RENDER_XRAY: AddDebugRenderObjectToVector( obj, s_debugRenderWorldOutlineObjectsXRay ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldWireBounds( const OBB3& bounds, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldWireBounds( bounds, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldWireBounds( const AABB3& bounds, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendVertsForAABB3D( vertices, bounds, color );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, color, color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: s_debugRenderWorldOutlineObjects.push_back( obj ); return;
		case DEBUG_RENDER_ALWAYS: s_debugRenderWorldOutlineObjectsAlways.push_back( obj ); return;
		case DEBUG_RENDER_XRAY: s_debugRenderWorldOutlineObjectsXRay.push_back( obj ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldWireSphere( const Vec3& pos, float radius, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendVertsAndIndicesForSphereMesh( vertices, indices, pos, radius, 16, 16, Rgba8::WHITE );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: AddDebugRenderObjectToVector( obj, s_debugRenderWorldOutlineObjects ); return;
		case DEBUG_RENDER_ALWAYS: AddDebugRenderObjectToVector( obj, s_debugRenderWorldOutlineObjectsAlways ); return;
		case DEBUG_RENDER_XRAY: AddDebugRenderObjectToVector( obj, s_debugRenderWorldOutlineObjectsXRay ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldWireSphere( const Vec3& pos, float radius, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldWireSphere( pos, radius, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldBasis( const Mat44& basis, const Rgba8& start_tint, const Rgba8& end_tint, float duration, eDebugRenderMode mode )
{
	DebugAddWorldArrow( basis.GetTranslation3D(), Rgba8::RED, Rgba8::RED, basis.GetTranslation3D() + basis.GetIBasis3D().GetNormalized(), start_tint, end_tint, duration, mode );
	DebugAddWorldArrow( basis.GetTranslation3D(), Rgba8::GREEN, Rgba8::GREEN, basis.GetTranslation3D() + basis.GetJBasis3D().GetNormalized(), start_tint, end_tint, duration, mode );
	DebugAddWorldArrow( basis.GetTranslation3D(), Rgba8::BLUE, Rgba8::BLUE, basis.GetTranslation3D() + basis.GetKBasis3D().GetNormalized(), start_tint, end_tint, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldBasis( const Mat44& basis, float duration, eDebugRenderMode mode )
{
	DebugAddWorldBasis( basis, Rgba8::WHITE, Rgba8::WHITE, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldText( const Mat44& basis, const Vec2& pivot, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode, char const* text )
{
	if ( text == nullptr
		 || strlen( text ) == 0 )
	{
		return;
	}

	BitmapFont* font = s_debugRenderContext->GetSystemFont();
	
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec2 textDimensions = font->GetDimensionsForText2D( .5f, text );
	Vec2 textMins = -pivot * textDimensions;

	font->AppendVertsAndIndicesForText2D( vertices, indices, textMins, .5f, text, Rgba8::WHITE );

	for ( int vertIdx = 0; vertIdx < (int)vertices.size(); ++vertIdx )
	{
		vertices[vertIdx].m_position = basis.TransformPosition3D( vertices[vertIdx].m_position );
	}

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: AddDebugRenderObjectToVector( obj, s_debugRenderWorldTextObjects ); return;
		case DEBUG_RENDER_ALWAYS: AddDebugRenderObjectToVector( obj, s_debugRenderWorldTextObjectsAlways ); return;
		case DEBUG_RENDER_XRAY: AddDebugRenderObjectToVector( obj, s_debugRenderWorldTextObjectsXRay ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldTextf( const Mat44& basis, const Vec2& pivot, const Rgba8& color, float duration, eDebugRenderMode mode, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddWorldText( basis, pivot, color, color, duration, mode, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldTextf( const Mat44& basis, const Vec2& pivot, const Rgba8& color, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddWorldText( basis, pivot, color, color, 0.f, DEBUG_RENDER_USE_DEPTH, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldBillboardText( const Vec3& origin, const Vec2& pivot, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode, char const* text )
{
	if ( text == nullptr
		 || strlen( text ) == 0 )
	{
		return;
	}

	BitmapFont* font = s_debugRenderContext->GetSystemFont();

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec2 textDimensions = font->GetDimensionsForText2D( .5f, text );
	Vec2 textMins = -pivot * textDimensions;

	font->AppendVertsAndIndicesForText2D( vertices, indices, textMins, .5f, text, Rgba8::WHITE );

	for ( int vertIdx = 0; vertIdx < (int)vertices.size(); ++vertIdx )
	{
		vertices[vertIdx].m_position += origin;
	}

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, origin, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: AddDebugRenderObjectToVector( obj, s_debugRenderWorldBillboardTextObjects ); return;
		case DEBUG_RENDER_ALWAYS: AddDebugRenderObjectToVector( obj, s_debugRenderWorldBillboardTextObjectsAlways ); return;
		case DEBUG_RENDER_XRAY: AddDebugRenderObjectToVector( obj, s_debugRenderWorldBillboardTextObjectsXRay ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldBillboardTextf( const Vec3& origin, const Vec2& pivot, const Rgba8& color, float duration, eDebugRenderMode mode, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddWorldBillboardText( origin, pivot, color, color, duration, mode, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldBillboardTextf( const Vec3& origin, const Vec2& pivot, const Rgba8& color, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddWorldBillboardText( origin, pivot, color, color, 0.f, DEBUG_RENDER_USE_DEPTH, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void DebugRenderSetScreenHeight( float height )
{
	s_screenHeight = height;
}


//-----------------------------------------------------------------------------------------------
AABB2 DebugGetScreenBounds()
{
	return AABB2( Vec2::ZERO, Vec2( s_screenWidth, s_screenHeight ) );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenPoint( const Vec2& pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration )
{
	std::vector<Vertex_PCU> vertices;
	
	AppendVertsForArc( vertices, pos, size, 360.f, 0.f, Rgba8::WHITE );
	
	DebugRenderObject* obj = new DebugRenderObject( vertices, start_color, end_color, duration );

	AddDebugRenderObjectToVector( obj, s_debugRenderScreenObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenPoint( const Vec2& pos, float size, const Rgba8& color, float duration )
{
	DebugAddScreenPoint( pos, size, color, color, duration );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenPoint( const Vec2& pos, const Rgba8& color )
{
	DebugAddScreenPoint( pos, 1.f, color, 0.f );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenLine( const Vec2& p0, const Rgba8& p0_start_color, const Vec2& p1, const Rgba8& p1_start_color, const Rgba8& start_tint, const Rgba8& end_tint, float duration )
{
	// Only support 1 color for now
	UNUSED( p1_start_color );

	std::vector<Vertex_PCU> vertices;

	Vec2 bone( p1 - p0 );
	Vec2 center( p0 + bone * .5f );
	Vec2 fullDimensions( bone.GetLength(), 5.f );
	Vec2 iBasis = bone.GetNormalized();
	OBB2 bounds( center, fullDimensions, iBasis );

	AppendVertsForOBB2D( vertices, bounds, p0_start_color );

	DebugRenderObject* obj = new DebugRenderObject( vertices, start_tint, end_tint, duration );

	AddDebugRenderObjectToVector( obj, s_debugRenderScreenObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenLine( const Vec2& p0, const Vec2& p1, const Rgba8& color, float duration )
{
	DebugAddScreenLine( p0, color, p1, color, color, color, duration );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenArrow( const Vec2& p0, const Rgba8& p0_start_color, const Vec2& p1, const Rgba8& p1_start_color, const Rgba8& start_tint, const Rgba8& end_tint, float duration )
{
	// Only support single colored arrows for now
	UNUSED( p1_start_color );

	const float arrowTipRatio = .05f;

	Vec2 bone( p1 - p0 );
	Vec2 lineDimensions( bone.GetLength(), 5.f );
	Vec2 iBasis = bone.GetNormalized();
	Vec2 jBasis = iBasis.GetRotatedMinus90Degrees();

	Vec2 endOfLine = lineDimensions.x * iBasis * ( 1.f - arrowTipRatio );

	std::vector<Vertex_PCU> vertices;
	vertices.push_back( Vertex_PCU( p0 + endOfLine + 10.f * jBasis, p0_start_color ) );
	vertices.push_back( Vertex_PCU( p1, p0_start_color ) );
	vertices.push_back( Vertex_PCU( p0 + endOfLine - 10.f * jBasis, p0_start_color ) );


	Vec2 center( p0 + bone * .5f );
	lineDimensions.x *= 1.f - arrowTipRatio;
	OBB2 bounds( center, lineDimensions, iBasis );

	AppendVertsForOBB2D( vertices, bounds, p0_start_color );

	DebugRenderObject* obj = new DebugRenderObject( vertices, start_tint, end_tint, duration );

	AddDebugRenderObjectToVector( obj, s_debugRenderScreenObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenArrow( const Vec2& p0, const Vec2& p1, const Rgba8& color, float duration )
{
	DebugAddScreenArrow( p0, color, p1, color, color, color, duration );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenQuad( const AABB2& bounds, const Rgba8& start_color, const Rgba8& end_color, float duration )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForAABB2D( vertices, bounds, Rgba8::WHITE );

	DebugRenderObject* obj = new DebugRenderObject( vertices, start_color, end_color, duration );

	AddDebugRenderObjectToVector( obj, s_debugRenderScreenObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenQuad( const AABB2& bounds, const Rgba8& color, float duration )
{
	DebugAddScreenQuad( bounds, color, color, duration );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenTexturedQuad( const AABB2& bounds, Texture* tex, const AABB2& uvs, const Rgba8& start_tint, const Rgba8& end_tint, float duration )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForAABB2D( vertices, bounds, Rgba8::WHITE, uvs.mins, uvs.maxs );

	DebugRenderObject* obj = new DebugRenderObject( vertices, start_tint, end_tint, tex, duration );

	AddDebugRenderObjectToVector( obj, s_debugRenderScreenTexturedObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenTexturedQuad( const AABB2& bounds, Texture* tex, const AABB2& uvs, const Rgba8& tint, float duration )
{
	DebugAddScreenTexturedQuad( bounds, tex, uvs, tint, tint, duration );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenTexturedQuad( const AABB2& bounds, Texture* tex, const Rgba8& tint, float duration )
{
	DebugAddScreenTexturedQuad( bounds, tex, AABB2::ZERO_TO_ONE, tint, tint, duration );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenText( const Vec4& pos, const Vec2& pivot, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, char const* text )
{
	if ( text == nullptr
		 || strlen( text ) == 0 )
	{
		return;
	}

	BitmapFont* font = s_debugRenderContext->GetSystemFont();
	
	std::vector<Vertex_PCU> vertices;

	Vec2 textDimensions = font->GetDimensionsForText2D( size, text );
	Vec2 textMins = -pivot * textDimensions;

	font->AppendVertsForText2D( vertices, textMins, size, text, Rgba8::WHITE );

	for ( int vertIdx = 0; vertIdx < (int)vertices.size(); ++vertIdx )
	{
		vertices[vertIdx].m_position += Vec3( pos.XY() * DebugGetScreenBounds().GetDimensions(), 0.f );
		vertices[vertIdx].m_position += Vec3( pos.ZW(), 0.f );
	}

	DebugRenderObject* obj = new DebugRenderObject( vertices, start_color, end_color, duration );

	AddDebugRenderObjectToVector( obj, s_debugRenderScreenTextObjects );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenTextf( const Vec4& pos, const Vec2& pivot, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddScreenText( pos, pivot, size, start_color, end_color, duration, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenTextf( const Vec4& pos, const Vec2& pivot, float size, const Rgba8& color, float duration, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddScreenText( pos, pivot, size, color, color, duration, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenTextf( const Vec4& pos, const Vec2& pivot, float size, const Rgba8& color, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddScreenText( pos, pivot, size, color, color, 0.f, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
void DebugAddScreenTextf( const Vec4& pos, const Vec2& pivot, const Rgba8& color, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddScreenText( pos, pivot, 10.f, color, color, 0.f, text.c_str() );
}

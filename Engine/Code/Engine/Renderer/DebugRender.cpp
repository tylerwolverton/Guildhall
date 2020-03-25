#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
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
static float s_screenHeight = 1080.f;


//-----------------------------------------------------------------------------------------------
// Private classes
//-----------------------------------------------------------------------------------------------
class DebugRenderObject
{
public:
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const Rgba8& startColor, const Rgba8& endColor, float duration = 0.f, const Vec3& origin = Vec3::ZERO );
	DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<uint>& indices, const Rgba8& startColor, const Rgba8& endColor, float duration = 0.f, const Vec3& origin = Vec3::ZERO );
	
	bool IsReadyToBeCulled() const;

	std::vector<Vertex_PCU> GetVertices() const { return m_vertices; }

public:
	std::vector<Vertex_PCU> m_vertices;
	std::vector<uint> m_indices;
	float m_duration = 0.f;
	Vec3 m_origin = Vec3::ZERO;
	Rgba8 m_startColor = Rgba8::MAGENTA;
	Rgba8 m_endColor = Rgba8::BLACK;
	Timer m_timer;
};


//-----------------------------------------------------------------------------------------------
DebugRenderObject::DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const Rgba8& startColor, const Rgba8& endColor, float duration, const Vec3& origin )
	: m_vertices( vertices )
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
DebugRenderObject::DebugRenderObject( const std::vector<Vertex_PCU>& vertices, const std::vector<uint>& indices, const Rgba8& startColor, const Rgba8& endColor, float duration, const Vec3& origin )
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
}


//-----------------------------------------------------------------------------------------------
void DebugRenderBeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
void InitializeDebugCamera( Camera* camera )
{
	s_debugCamera->SetClearMode( CLEAR_NONE );
	s_debugCamera->SetTransform( camera->GetTransform() );
	s_debugCamera->SetColorTarget( camera->GetColorTarget() );
	s_debugCamera->SetOutputSize( camera->GetOutputSize() );
	s_debugCamera->SetDepthStencilTarget( camera->GetDepthStencilTarget() );
	s_debugCamera->SetViewMatrix( camera->GetViewMatrix() );
	s_debugCamera->SetProjectionMatrix( camera->GetProjectionMatrix() );
}


//-----------------------------------------------------------------------------------------------
void RenderWorldObjects( const std::vector<DebugRenderObject*> objects )
{
	for ( int debugObjIdx = 0; debugObjIdx < (int)objects.size(); ++debugObjIdx )
	{
		DebugRenderObject*const& obj = objects[debugObjIdx];
		if ( obj != nullptr )
		{
			std::vector<Vertex_PCU> vertices;
			std::vector<uint> indices;

			AppendDebugObjectToVertexArray( vertices, indices, obj );

			GPUMesh mesh( s_debugRenderContext, vertices, indices );
			s_debugRenderContext->DrawMesh( &mesh );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void RenderWorldBillboardTextObjects( const std::vector<DebugRenderObject*> objects )
{
	for ( int debugObjIdx = 0; debugObjIdx < (int)objects.size(); ++debugObjIdx )
	{
		DebugRenderObject*const& obj = objects[debugObjIdx];
		if ( obj != nullptr )
		{
			std::vector<Vertex_PCU> vertices;
			std::vector<uint> indices;

			AppendDebugObjectToVertexArray( vertices, indices, obj );

			Vec3 cameraRotation = s_debugCamera->GetTransform().m_rotation;
			Mat44 rotationMatrix = Mat44::CreateRotationFromPitchRollYawDegrees( cameraRotation.x, cameraRotation.y, cameraRotation.z );
			//Mat44 model = s_debugCamera->GetProjectionMatrix();
			//model.SetTranslation3D( obj->m_origin );

			Mat44 modelMatrix = Mat44::CreateTranslation3D( obj->m_origin );
			Mat44 inverseTranslationMatrix = Mat44::CreateTranslation3D( -obj->m_origin );
			modelMatrix.PushTransform( rotationMatrix );
			modelMatrix.PushTransform( inverseTranslationMatrix );

			for ( int vertIdx = 0; vertIdx < (int)vertices.size(); ++vertIdx )
			{
				vertices[vertIdx].m_position = modelMatrix.TransformPosition3D( vertices[vertIdx].m_position );
			}

			GPUMesh mesh( s_debugRenderContext, vertices, indices );
			//s_debugRenderContext->SetModelMatrix( modelMatrix );
			s_debugRenderContext->DrawMesh( &mesh );
		}
	}
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

	BitmapFont* font = s_debugRenderContext->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	// Draw Depth
	s_debugRenderContext->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );
	s_debugRenderContext->BindTexture( nullptr );
	RenderWorldObjects( s_debugRenderWorldObjects );
	RenderWorldObjects( s_debugRenderWorldObjectsXRay );
	
	s_debugRenderContext->SetFillMode( eFillMode::WIREFRAME );
	RenderWorldObjects( s_debugRenderWorldOutlineObjects );
	RenderWorldObjects( s_debugRenderWorldOutlineObjectsXRay );
	s_debugRenderContext->SetFillMode( eFillMode::SOLID );
	
	s_debugRenderContext->BindTexture( font->GetTexture() );
	RenderWorldObjects( s_debugRenderWorldTextObjects );
	RenderWorldObjects( s_debugRenderWorldTextObjectsXRay );
	RenderWorldBillboardTextObjects( s_debugRenderWorldBillboardTextObjects );
	RenderWorldBillboardTextObjects( s_debugRenderWorldBillboardTextObjectsXRay );

	// Draw Always
	s_debugRenderContext->SetDepthTest( eCompareFunc::COMPARISON_ALWAYS, false );

	s_debugRenderContext->BindTexture( nullptr );
	RenderWorldObjects( s_debugRenderWorldObjectsAlways );

	s_debugRenderContext->SetFillMode( eFillMode::WIREFRAME );
	RenderWorldObjects( s_debugRenderWorldOutlineObjectsAlways );
	s_debugRenderContext->SetFillMode( eFillMode::SOLID );

	s_debugRenderContext->BindTexture( font->GetTexture() );
	RenderWorldObjects( s_debugRenderWorldTextObjectsAlways );
	RenderWorldBillboardTextObjects( s_debugRenderWorldBillboardTextObjectsAlways );

	// Draw XRay
	s_debugRenderContext->SetDepthTest( eCompareFunc::COMPARISON_GREATER, false );
	s_debugRenderContext->BindShader( "Data/Shaders/XRay.hlsl" );

	s_debugRenderContext->BindTexture( nullptr );
	RenderWorldObjects( s_debugRenderWorldObjectsXRay );

	s_debugRenderContext->SetFillMode( eFillMode::WIREFRAME );
	RenderWorldObjects( s_debugRenderWorldOutlineObjectsXRay );
	s_debugRenderContext->SetFillMode( eFillMode::SOLID );

	s_debugRenderContext->BindTexture( font->GetTexture() );
	RenderWorldObjects( s_debugRenderWorldTextObjectsXRay );
	RenderWorldBillboardTextObjects( s_debugRenderWorldBillboardTextObjectsXRay );

	s_debugRenderContext->BindShader( "Data/Shaders/Default.hlsl" );

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
void CullExpiredObjects( std::vector<DebugRenderObject*>& objects )
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
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldPoint( const Vec3& pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode )
{
	UNUSED( mode );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AABB3 pointBounds( Vec3::ZERO, Vec3( size, size, size ) );
	pointBounds.SetCenter( pos );
		
	AppendVertsForCubeMesh( vertices, pos, size, start_color );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );
	
	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: s_debugRenderWorldObjects.push_back( obj ); return;
		case DEBUG_RENDER_ALWAYS: s_debugRenderWorldObjectsAlways.push_back( obj ); return;
		case DEBUG_RENDER_XRAY: s_debugRenderWorldObjectsXRay.push_back( obj ); return;
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

	Mat44 lookAt = MakeLookAtMatrix( p0, p1 );

	Vec3 obbCenter = p0 + ( obbBone * .5f );
	Vec3 obbDimensions( .01f, .01f, obbBone.GetLength() );

	OBB3 lineBounds( obbCenter, obbDimensions, lookAt.GetIBasis3D(), lookAt.GetJBasis3D() );

	AppendVertsForOBB3D( vertices, lineBounds, p0_start_color );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, p0_start_color, p0_end_color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: s_debugRenderWorldObjects.push_back( obj ); return;
		case DEBUG_RENDER_ALWAYS: s_debugRenderWorldObjectsAlways.push_back( obj ); return;
		case DEBUG_RENDER_XRAY: s_debugRenderWorldObjectsXRay.push_back( obj ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldLine( const Vec3& start, const Vec3& end, const Rgba8& color, float duration, eDebugRenderMode mode )
{
	DebugAddWorldLine( start, color, color, end, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldWireBounds( const OBB3& bounds, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode )
{
	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	AppendVertsForOBB3D( vertices, bounds, start_color );
	AppendIndicesForCubeMesh( indices );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: s_debugRenderWorldOutlineObjects.push_back( obj ); return;
		case DEBUG_RENDER_ALWAYS: s_debugRenderWorldOutlineObjectsAlways.push_back( obj ); return;
		case DEBUG_RENDER_XRAY: s_debugRenderWorldOutlineObjectsXRay.push_back( obj ); return;
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

	AppendVertsAndIndicesForSphereMesh( vertices, indices, pos, radius, 16, 16, start_color );

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: s_debugRenderWorldOutlineObjects.push_back( obj ); return;
		case DEBUG_RENDER_ALWAYS: s_debugRenderWorldOutlineObjectsAlways.push_back( obj ); return;
		case DEBUG_RENDER_XRAY: s_debugRenderWorldOutlineObjectsXRay.push_back( obj ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldWireSphere( const Vec3& pos, float radius, const Rgba8& color, float duration /*= 0.0f*/, eDebugRenderMode mode /*= DEBUG_RENDER_USE_DEPTH */ )
{
	DebugAddWorldWireSphere( pos, radius, color, color, duration, mode );
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldText( const Mat44& basis, const Vec2& pivot, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode, char const* text )
{
	if ( text == nullptr
		 || strlen( text ) == 0 )
	{
		return;
	}

	// TODO: fall back to triangle font if none found?
	BitmapFont* font = s_debugRenderContext->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	UNUSED( mode );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec2 textDimensions = font->GetDimensionsForText2D( .5f, text );
	Vec2 textMins = -pivot * textDimensions;

	font->AppendVertsAndIndicesForText2D( vertices, indices, textMins, .5f, text, start_color );

	for ( int vertIdx = 0; vertIdx < (int)vertices.size(); ++vertIdx )
	{
		vertices[vertIdx].m_position = basis.TransformPosition3D( vertices[vertIdx].m_position );
	}

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: s_debugRenderWorldTextObjects.push_back( obj ); return;
		case DEBUG_RENDER_ALWAYS: s_debugRenderWorldTextObjectsAlways.push_back( obj ); return;
		case DEBUG_RENDER_XRAY: s_debugRenderWorldTextObjectsXRay.push_back( obj ); return;
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

	// TODO: fall back to triangle font if none found?
	BitmapFont* font = s_debugRenderContext->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	UNUSED( mode );

	std::vector<Vertex_PCU> vertices;
	std::vector<uint> indices;

	Vec2 textDimensions = font->GetDimensionsForText2D( .5f, text );
	Vec2 textMins = -pivot * textDimensions;

	font->AppendVertsAndIndicesForText2D( vertices, indices, textMins, .5f, text, start_color );

	for ( int vertIdx = 0; vertIdx < (int)vertices.size(); ++vertIdx )
	{
		vertices[vertIdx].m_position += origin;
	}

	DebugRenderObject* obj = new DebugRenderObject( vertices, indices, start_color, end_color, duration, origin );

	switch ( mode )
	{
		case DEBUG_RENDER_USE_DEPTH: s_debugRenderWorldBillboardTextObjects.push_back( obj ); return;
		case DEBUG_RENDER_ALWAYS: s_debugRenderWorldBillboardTextObjectsAlways.push_back( obj ); return;
		case DEBUG_RENDER_XRAY: s_debugRenderWorldBillboardTextObjectsXRay.push_back( obj ); return;
	}
}


//-----------------------------------------------------------------------------------------------
void DebugAddWorldBillboardTextf( const Vec3& origin, const Vec2& pivot, const Rgba8& color, float duration, eDebugRenderMode mode, char const* format, ... )
{
	va_list args;
	va_start( args, format );
	std::string text = Stringv( format, args );

	DebugAddWorldBillboardText( origin, pivot, color, color, duration, DEBUG_RENDER_USE_DEPTH, text.c_str() );
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

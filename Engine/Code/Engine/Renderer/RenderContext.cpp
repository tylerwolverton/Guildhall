#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Polygon2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/BuiltInShaders.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/OS/Window.hpp"

#include "ThirdParty/stb/stb_image.h"


//-----------------------------------------------------------------------------------------------
// DX3D11 Includes

#pragma comment( lib, "d3d11.lib" )         // needed a01
#pragma comment( lib, "dxgi.lib" )          // needed a01
#pragma comment( lib, "d3dcompiler.lib" )   // needed when we get to shaders


//-----------------------------------------------------------------------------------------------
void RenderContext::Startup( Window* window )
{
	IDXGISwapChain* swapchain = nullptr;
	
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
	#if defined(RENDER_DEBUG)
		flags |= D3D11_CREATE_DEVICE_DEBUG;
		CreateDebugModule();
	#endif

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	memset( &swapchainDesc, 0, sizeof( swapchainDesc ) );

	// how many back buffers in our chain - we'll double buffer (one we show, one we draw to)
	swapchainDesc.BufferCount = 2;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // on swap, the old buffer is discarded
	swapchainDesc.Flags = 0; // additional flags - see docs.  Used in special cases like for video buffers

	// how swap chain is to be used
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	HWND hwnd = (HWND)window->m_hwnd;
	swapchainDesc.OutputWindow = hwnd;		// HWND for the window to be used
	swapchainDesc.SampleDesc.Count = 1;		// how many samples per pixel (1 so no MSAA)
											// note, if we're doing MSAA, we'll do it on a secondary target

	// describe the buffer
	swapchainDesc.Windowed = TRUE;                                    // windowed/full-screen mode
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color RGBA8 color
	swapchainDesc.BufferDesc.Width = window->GetClientWidth();
	swapchainDesc.BufferDesc.Height = window->GetClientHeight();

	HRESULT result = D3D11CreateDeviceAndSwapChain( nullptr,
								   D3D_DRIVER_TYPE_HARDWARE,
								   nullptr,
								   flags, // controls the type of device we make
								   nullptr,
								   0,
								   D3D11_SDK_VERSION,
								   &swapchainDesc,
								   &swapchain,
								   &m_device,
								   nullptr,
								   &m_context );

	GUARANTEE_OR_DIE( SUCCEEDED( result ), "Failed to create rendering device." );

	if ( swapchain != nullptr ) 
	{
		m_swapchain = new SwapChain( this, swapchain );
	}

	// Create default shader
	m_defaultShader = GetOrCreateShaderFromSourceString( "DefaultBuiltInShader", g_defaultShaderCode );

	m_immediateVBO = new VertexBuffer( this, MEMORY_HINT_DYNAMIC );
	m_immediateIBO = new IndexBuffer( this, MEMORY_HINT_DYNAMIC );
	m_frameUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_modelMatrixUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );

	m_defaultPointSampler = new Sampler( this, SAMPLER_POINT );
	m_defaultLinearSampler = new Sampler( this, SAMPLER_BILINEAR );
	m_currentSampler = m_defaultPointSampler;

	m_defaultWhiteTexture = CreateTextureFromColor( Rgba8::WHITE );

	CreateBlendStates();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{
}


//-----------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	m_swapchain->Present();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::Shutdown()
{
	delete m_immediateVBO;
	delete m_immediateIBO;
	delete m_frameUBO;
	delete m_modelMatrixUBO;

	delete m_defaultPointSampler;
	m_defaultPointSampler = nullptr;

	delete m_defaultLinearSampler;
	m_defaultLinearSampler = nullptr;

	delete m_defaultWhiteTexture;
	m_defaultWhiteTexture = nullptr;

	DX_SAFE_RELEASE( m_alphaBlendState );
	DX_SAFE_RELEASE( m_additiveBlendState );
	DX_SAFE_RELEASE( m_disabledBlendState );

	// Cleanup shader cache
	for ( int shaderIdx = 0; shaderIdx < (int)m_loadedShaders.size(); ++shaderIdx )
	{
		delete m_loadedShaders[shaderIdx];
		m_loadedShaders[shaderIdx] = nullptr;
	}
	m_loadedShaders.clear();

	// Cleanup bitmap font cache
	for ( int fontIndex = 0; fontIndex < (int)m_loadedBitmapFonts.size(); ++fontIndex )
	{
		delete m_loadedBitmapFonts[fontIndex];
		m_loadedBitmapFonts[fontIndex] = nullptr;
	}
	m_loadedBitmapFonts.clear();

	// Cleanup texture cache
	for ( int textureIndex = 0; textureIndex < (int)m_loadedTextures.size(); ++textureIndex )
	{
		delete m_loadedTextures[textureIndex];
		m_loadedTextures[textureIndex] = nullptr;
	}
	m_loadedTextures.clear();

	delete m_swapchain;
	m_swapchain = nullptr;

	// release
	DX_SAFE_RELEASE( m_context );
	DX_SAFE_RELEASE( m_device );

	// do our leak reporting just before shutdown to give us a better detailed list
	ReportLiveObjects();    
	DestroyDebugModule();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetBlendMode( eBlendMode blendMode )
{
	float const zeroes[] = { 0,0,0,0 };

	switch ( blendMode )
	{
		case eBlendMode::ALPHA: m_context->OMSetBlendState( m_alphaBlendState, zeroes, ~0U ); m_currentBlendMode = eBlendMode::ALPHA; return;
		case eBlendMode::ADDITIVE: m_context->OMSetBlendState( m_additiveBlendState, zeroes, ~0U ); m_currentBlendMode = eBlendMode::ADDITIVE; return;
		case eBlendMode::DISABLED: m_context->OMSetBlendState( m_disabledBlendState, zeroes, ~0U ); m_currentBlendMode = eBlendMode::DISABLED; return;
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::ClearScreen( ID3D11RenderTargetView* renderTargetView, const Rgba8& clearColor )
{
	float clearFloats[4];
	clearFloats[0] = (float)clearColor.r / 255.f;
	clearFloats[1] = (float)clearColor.g / 255.f;
	clearFloats[2] = (float)clearColor.b / 255.f;
	clearFloats[3] = (float)clearColor.a / 255.f;

	m_context->ClearRenderTargetView( renderTargetView, clearFloats );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BeginCamera( Camera& camera )
{
	#if defined(RENDER_DEBUG)
		m_context->ClearState();
	#endif

	m_isDrawing = true;

	Texture* colorTarget = camera.GetColorTarget();
	if ( colorTarget == nullptr )
	{
		colorTarget = m_swapchain->GetBackBuffer();
	}

	if ( camera.m_cameraUBO == nullptr )
	{
		camera.m_cameraUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	}
	
	camera.UpdateCameraUBO();
	SetModelMatrix( Mat44() );

	BindUniformBuffer( UBO_FRAME_SLOT, m_frameUBO );
	BindUniformBuffer( UBO_CAMERA_SLOT, camera.m_cameraUBO );
	BindUniformBuffer( UBO_MODEL_MATRIX_SLOT, m_modelMatrixUBO );

	// Viewport creation
	TextureView* view = colorTarget->GetOrCreateRenderTargetView();
	ID3D11RenderTargetView* renderTargetView = view->m_renderTargetView;
	m_context->OMSetRenderTargets( 1, &renderTargetView, nullptr );

	IntVec2 outputSize = colorTarget->GetTexelSize();

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)outputSize.x;
	viewport.Height = (float)outputSize.y;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_context->RSSetViewports( 1, &viewport );
		
	if ( camera.GetClearMode() & eCameraClearBitFlag::CLEAR_COLOR_BIT )
	{
		ClearScreen( renderTargetView, camera.GetClearColor() );
	}

	// Reset
	BindShader( (Shader*)nullptr );
	BindTexture( nullptr );
	BindSampler( nullptr );
	m_lastVBOHandle = nullptr;
	m_lastIBOHandle = nullptr;

	SetBlendMode( m_currentBlendMode );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::EndCamera( const Camera& camera )
{
	UNUSED( camera );
	m_isDrawing = false;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::UpdateFrameTime( float deltaSeconds )
{
	FrameData frameData;
	frameData.systemTimeSeconds = (float)GetCurrentTimeSeconds();
	frameData.systemDeltaTimeSeconds = deltaSeconds;

	m_frameUBO->Update( &frameData, sizeof( frameData ), sizeof( frameData ) );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::Draw( int numVertices, int vertexOffset )
{
	// Describe Vertex Format to Shader
	ID3D11InputLayout* inputLayout = m_currentShader->GetOrCreateInputLayout( m_immediateVBO->m_attributes );
	m_context->IASetInputLayout( inputLayout );

	m_context->Draw( numVertices, vertexOffset );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( int numVertices, const Vertex_PCU* vertices )
{
	// Update a vertex buffer
	size_t dataByteSize = numVertices * sizeof( Vertex_PCU );
	size_t elementSize = sizeof( Vertex_PCU );
	m_immediateVBO->Update( vertices, dataByteSize, elementSize );

	// Bind
	BindVertexBuffer( m_immediateVBO );

	// Draw
	Draw( numVertices, 0 );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( const std::vector<Vertex_PCU>& vertices )
{
	GUARANTEE_OR_DIE( vertices.size() > 0, "Empty vertex array cannot be drawn" );
	DrawVertexArray( (int)vertices.size(), &vertices[0] );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawIndexed( int indexCount, int indexOffset, int vertexOffset )
{
	m_context->DrawIndexed( indexCount, indexOffset, vertexOffset );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawMesh( GPUMesh* mesh )
{	
	// Bind
	BindVertexBuffer( mesh->m_vertices );

	// Describe Vertex Format to Shader
	// UpdateLayoutFunctionIfNeeded
	ID3D11InputLayout* inputLayout = m_currentShader->GetOrCreateInputLayout( mesh->m_vertices->m_attributes );
	m_context->IASetInputLayout( inputLayout );

	// Draw
	if ( mesh->GetIndexCount() > 0 )
	{
		BindIndexBuffer( mesh->m_indices );
		DrawIndexed( mesh->GetIndexCount() );
	}
	else
	{
		Draw( mesh->GetVertexCount() );
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawLine2D( const Vec2& start, const Vec2& end, const Rgba8& color, float thickness )
{
	float radius = 0.5f * thickness;
	Vec2 displacement = end - start;

	// Create a small vector to be used to add a little
	// extra to the end of each line to make overlapping look better
	Vec2 forward = displacement.GetNormalized();
	forward *= radius;

	Vec2 left = forward.GetRotated90Degrees();

	// Calculate each corner of the box that will represent the line
	Vec2 startLeft = start - forward + left;
	Vec2 startRight = start - forward - left;
	Vec2 endLeft = end + forward + left;
	Vec2 endRight = end + forward - left;

	Vertex_PCU lineVertices[] =
	{
		Vertex_PCU( startRight, color ),
		Vertex_PCU( endRight, color ),
		Vertex_PCU( endLeft, color ),

		Vertex_PCU( startRight, color ),
		Vertex_PCU( endLeft, color ),
		Vertex_PCU( startLeft, color )
	};

	constexpr int NUM_VERTICES = sizeof( lineVertices ) / sizeof( lineVertices[0] );
	DrawVertexArray( NUM_VERTICES, lineVertices );
}


//-----------------------------------------------------------------------------------------------
// Render a ring as 64 small lines
//
void RenderContext::DrawRing2D( const Vec2& center, float radius, const Rgba8& color, float thickness )
{
	constexpr float NUM_SIDES = 64.f;
	constexpr float DEG_PER_SIDE = 360.f / NUM_SIDES;

	for ( int sideIndex = 0; sideIndex < NUM_SIDES; ++sideIndex )
	{
		float thetaDeg = DEG_PER_SIDE * (float)sideIndex;
		float theta2Deg = DEG_PER_SIDE * (float)( sideIndex + 1 );

		Vec2 start( radius * CosDegrees( thetaDeg ),
					radius * SinDegrees( thetaDeg ) );

		Vec2 end( radius * CosDegrees( theta2Deg ),
				  radius * SinDegrees( theta2Deg ) );

		// Translate start and end to be relative to the center of the ring
		start += center;
		end += center;

		DrawLine2D( start, end, color, thickness );
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawDisc2D( const Vec2& center, float radius, const Rgba8& color )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForArc( vertices, center, radius, 360.f, 0.f, color );

	DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawCapsule2D( const Capsule2& capsule, const Rgba8& color )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForCapsule2D( vertices, capsule, color );

	DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawAABB2( const AABB2& box, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForAABB2D( vertices, box, tint );

	DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawAABB2WithDepth( const AABB2& box, float zDepth, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForAABB2DWithDepth( vertices, box, zDepth, tint );

	DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawOBB2( const OBB2& box, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForOBB2D( vertices, box, tint );

	DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
// TODO: This doesn't draw at correct position
void RenderContext::DrawAABB2Outline( const Vec2& center, const AABB2& box, const Rgba8& tint, float thickness )
{
	Vec2 bottomLeft( box.mins + center );
	Vec2 bottomRight( Vec2( box.maxs.x, box.mins.y ) + center );
	Vec2 topLeft( Vec2( box.mins.x, box.maxs.y ) + center );
	Vec2 topRight( box.maxs + center );

	DrawLine2D( bottomLeft, bottomRight, tint, thickness );
	DrawLine2D( bottomLeft, topLeft,	 tint, thickness );
	DrawLine2D( topLeft,	topRight,	 tint, thickness );
	DrawLine2D( topRight,	bottomRight, tint, thickness );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawOBB2Outline( const Vec2& center, const OBB2& box, const Rgba8& tint, float thickness )
{
	Vec2 boxHalfWidth( box.m_halfDimensions.x * box.m_iBasis );
	Vec2 boxHalfHeight( box.m_halfDimensions.y * box.GetJBasisNormal() );

	Vec2 topRight(    center + box.m_center + boxHalfWidth + boxHalfHeight );
	Vec2 topLeft(	  center + box.m_center - boxHalfWidth + boxHalfHeight );
	Vec2 bottomLeft(  center + box.m_center - boxHalfWidth - boxHalfHeight );
	Vec2 bottomRight( center + box.m_center + boxHalfWidth - boxHalfHeight );

	DrawLine2D( bottomLeft, bottomRight, tint, thickness );
	DrawLine2D( bottomLeft, topLeft,	 tint, thickness );
	DrawLine2D( topLeft,	topRight,	 tint, thickness );
	DrawLine2D( topRight,	bottomRight, tint, thickness );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawPolygon2( const std::vector<Vec2>& vertexPositions, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForPolygon2( vertices, vertexPositions, tint );

	DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawPolygon2( const Polygon2& polygon2, const Rgba8& tint )
{
	DrawPolygon2( polygon2.GetPoints(), tint );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawPolygon2Outline( const std::vector<Vec2>& vertexPositions, const Rgba8& tint, float thickness )
{
	int numVertices = (int)vertexPositions.size();

	if ( numVertices < 3 )
	{
		g_devConsole->PrintString( Stringf( "Tried to draw a Polygon2Outline with %d vertices. At least 3 vertices are required.", numVertices ), Rgba8::YELLOW );
		return;
	}

	for ( int vertexPosIdx = 0; vertexPosIdx < numVertices - 1; ++vertexPosIdx )
	{
		int nextVertexIdx = vertexPosIdx + 1;
		DrawLine2D( vertexPositions[ vertexPosIdx ], vertexPositions[ nextVertexIdx ], tint, thickness );
	}

	// Connect last vertex to first
	int lastVertexIdx = numVertices - 1;
	DrawLine2D( vertexPositions[ lastVertexIdx ], vertexPositions[0], tint, thickness );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawPolygon2Outline( const Polygon2& polygon2, const Rgba8& tint, float thickness )
{
	DrawPolygon2Outline( polygon2.GetPoints(), tint, thickness );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendVertsForArc( std::vector<Vertex_PCU>& vertexArray, const Vec2& center, float radius, float arcAngleDegrees, float startOrientationDegrees, const Rgba8& tint )
{
	constexpr float NUM_SIDES = 64.f;
	float degreesPerSide = arcAngleDegrees / NUM_SIDES;

	for ( int segmentNum = 0; segmentNum < NUM_SIDES; ++segmentNum )
	{
		float thetaDeg = startOrientationDegrees + ( degreesPerSide * (float)segmentNum );
		float theta2Deg = startOrientationDegrees + ( degreesPerSide * (float)( segmentNum + 1 ) );

		Vec2 start( radius * CosDegrees( thetaDeg ),
					radius * SinDegrees( thetaDeg ) );

		Vec2 end( radius * CosDegrees( theta2Deg ),
				  radius * SinDegrees( theta2Deg ) );
		
		// Add triangle segment
		vertexArray.push_back( Vertex_PCU( center, tint ) );
		vertexArray.push_back( Vertex_PCU( center + start, tint ) );
		vertexArray.push_back( Vertex_PCU( center + end, tint ) );
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendVertsForAABB2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	vertexArray.push_back( Vertex_PCU( spriteBounds.mins,									tint,	uvAtMins) );
	vertexArray.push_back( Vertex_PCU( Vec2( spriteBounds.maxs.x, spriteBounds.mins.y ),	tint,	Vec2( uvAtMaxs.x, uvAtMins.y )) );
	vertexArray.push_back( Vertex_PCU( spriteBounds.maxs,									tint,	uvAtMaxs) );

	vertexArray.push_back( Vertex_PCU( spriteBounds.mins,									tint,	uvAtMins) );
	vertexArray.push_back( Vertex_PCU( spriteBounds.maxs,									tint,	uvAtMaxs) );
	vertexArray.push_back( Vertex_PCU( Vec2( spriteBounds.mins.x, spriteBounds.maxs.y ),	tint,	Vec2( uvAtMins.x, uvAtMaxs.y )) );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const OBB2& spriteBounds, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec2 boxHalfWidth( spriteBounds.m_halfDimensions.x * spriteBounds.m_iBasis );
	Vec2 boxHalfHeight( spriteBounds.m_halfDimensions.y * spriteBounds.GetJBasisNormal() );
	
	Vec2 topRight   ( spriteBounds.m_center + boxHalfWidth + boxHalfHeight );
	Vec2 topLeft    ( spriteBounds.m_center - boxHalfWidth + boxHalfHeight );
	Vec2 bottomLeft ( spriteBounds.m_center - boxHalfWidth - boxHalfHeight );
	Vec2 bottomRight( spriteBounds.m_center + boxHalfWidth - boxHalfHeight );
	
	AppendVertsForOBB2D( vertexArray, bottomLeft, bottomRight, topLeft, topRight, tint, uvAtMins, uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendVertsForOBB2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& bottomLeft, const Vec2& bottomRight, const Vec2& topLeft, const Vec2& topRight, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	vertexArray.push_back( Vertex_PCU( bottomLeft,	tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( bottomRight, tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( topRight,	tint, uvAtMaxs ) );

	vertexArray.push_back( Vertex_PCU( bottomLeft,	tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( topRight,	tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( topLeft,		tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendVertsForCapsule2D( std::vector<Vertex_PCU>& vertexArray, const Capsule2& capsule, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec2 iBasis = capsule.m_middleEnd - capsule.m_middleStart;
	iBasis.Normalize();
	iBasis.Rotate90Degrees();

	// Add middle box
	AppendVertsForOBB2D( vertexArray, capsule.GetCenterSectionAsOBB2(), tint, uvAtMins, uvAtMaxs );

	// Add end caps
	AppendVertsForArc( vertexArray, capsule.m_middleStart, capsule.m_radius, 180.f, iBasis.GetOrientationDegrees(), tint );
	AppendVertsForArc( vertexArray, capsule.m_middleEnd,   capsule.m_radius, 180.f, iBasis.GetRotatedDegrees(- 180.f ).GetOrientationDegrees(), tint );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendVertsForPolygon2( std::vector<Vertex_PCU>& vertexArray, const std::vector<Vec2>& vertexPositions, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	int numVertices = (int)vertexPositions.size();

	if ( numVertices < 3 )
	{
		g_devConsole->PrintString( Stringf( "Tried to append verts for a Polygon2 with %d vertices. At least 3 vertices are required.", numVertices ), Rgba8::YELLOW );
		return;
	}

	for ( int vertexPosIdx = 1; vertexPosIdx < numVertices - 1; ++vertexPosIdx )
	{
		int nextVertexIdx = vertexPosIdx + 1;
		vertexArray.push_back( Vertex_PCU( vertexPositions[0],			   tint, uvAtMins ) );
		vertexArray.push_back( Vertex_PCU( vertexPositions[vertexPosIdx], tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
		vertexArray.push_back( Vertex_PCU( vertexPositions[nextVertexIdx], tint, uvAtMaxs ) );
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendVertsForCubeMesh( std::vector<Vertex_PCU>& vertexArray, const Vec3& center, float sideLength, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs)
{
	Vec3 mins( center );
	mins.x -= sideLength * .5f;
	mins.y -= sideLength * .5f;
	mins.z += sideLength * .5f;

	Vec3 maxs( center );
	maxs.x += sideLength * .5f;
	maxs.y += sideLength * .5f;
	maxs.z -= sideLength * .5f;

	// Front 4 points
	Vertex_PCU vert0( mins, tint, uvAtMins );
	Vertex_PCU vert1( Vec3( maxs.x, mins.y, mins.z ), tint, Vec2( uvAtMaxs.x, uvAtMins.y ) );
	Vertex_PCU vert2( Vec3( mins.x, maxs.y, mins.z ), tint, Vec2( uvAtMins.x, uvAtMaxs.y ) );
	Vertex_PCU vert3( Vec3( maxs.x, maxs.y, mins.z ), tint, uvAtMaxs );

	Vec3 backMins( mins );
	backMins.z = center.z - sideLength * .5f;

	Vec3 backMaxs( maxs );
	backMaxs.z = center.z + sideLength * .5f;

	// Back 4 points ( from front perspective for directions )	
	Vertex_PCU vert4( backMins, tint, uvAtMins );
	Vertex_PCU vert5( Vec3( backMaxs.x, backMins.y, backMins.z ), tint, Vec2( uvAtMaxs.x, uvAtMins.y ) );
	Vertex_PCU vert6( Vec3( backMins.x, backMaxs.y, backMins.z ), tint, Vec2( uvAtMins.x, uvAtMaxs.y ) );
	Vertex_PCU vert7( Vec3( backMaxs.x, backMaxs.y, backMins.z ), tint, uvAtMaxs );

	vertexArray.reserve( 24 );
	// Front
	vertexArray.push_back( vert0 );
	vertexArray.push_back( vert1 );
	vertexArray.push_back( vert2 );
	vertexArray.push_back( vert3 );

	// Right
	vertexArray.push_back( vert1 );
	vertexArray.push_back( vert5 );
	vertexArray.push_back( vert3 );
	vertexArray.push_back( vert7 );

	// Back
	vertexArray.push_back( vert4 );
	vertexArray.push_back( vert5 );
	vertexArray.push_back( vert6 );
	vertexArray.push_back( vert7 );

	// Left
	vertexArray.push_back( vert4 );
	vertexArray.push_back( vert0 );
	vertexArray.push_back( vert6 );
	vertexArray.push_back( vert2 );

	// Top
	vertexArray.push_back( vert2 );
	vertexArray.push_back( vert3 );
	vertexArray.push_back( vert6 );
	vertexArray.push_back( vert7 );

	// Bottom
	vertexArray.push_back( vert0 );
	vertexArray.push_back( vert1 );
	vertexArray.push_back( vert4 );
	vertexArray.push_back( vert5 );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendIndicesForCubeMesh( std::vector<uint>& indices )
{
	indices.reserve( 36 );
	// Front face
	indices.push_back( 0 );
	indices.push_back( 1 );
	indices.push_back( 3 );

	indices.push_back( 0 );
	indices.push_back( 3 );
	indices.push_back( 2 );

	// Right face
	indices.push_back( 1 );
	indices.push_back( 5 );
	indices.push_back( 7 );
	
	indices.push_back( 1 );
	indices.push_back( 7 );
	indices.push_back( 3 );

	// Back face
	indices.push_back( 8 );
	indices.push_back( 9 );
	indices.push_back( 11 );
	
	indices.push_back( 8 );
	indices.push_back( 11 );
	indices.push_back( 10 );

	// Left face
	indices.push_back( 12 );
	indices.push_back( 13 );
	indices.push_back( 15 );
	
	indices.push_back( 12 );
	indices.push_back( 15 );
	indices.push_back( 14 );

	//// Top face
	//indices.push_back( 3 );
	//indices.push_back( 2 );
	//indices.push_back( 6 );
	//
	//indices.push_back( 3 );
	//indices.push_back( 6 );
	//indices.push_back( 7 );

	//// Bottom face
	//indices.push_back( 0 );
	//indices.push_back( 1 );
	//indices.push_back( 5 );
	//
	//indices.push_back( 0 );
	//indices.push_back( 5 );
	//indices.push_back( 4 );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::AppendVertsForAABB2DWithDepth( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds, float zDepth, const Rgba8& tint, const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	Vec3 mins( spriteBounds.mins, zDepth );
	Vec3 maxs( spriteBounds.maxs, zDepth );

	vertexArray.push_back( Vertex_PCU( mins, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( Vec3( spriteBounds.maxs.x, spriteBounds.mins.y, zDepth ), tint, Vec2( uvAtMaxs.x, uvAtMins.y ) ) );
	vertexArray.push_back( Vertex_PCU( maxs, tint, uvAtMaxs ) );

	vertexArray.push_back( Vertex_PCU( mins, tint, uvAtMins ) );
	vertexArray.push_back( Vertex_PCU( maxs, tint, uvAtMaxs ) );
	vertexArray.push_back( Vertex_PCU( Vec3( spriteBounds.mins.x, spriteBounds.maxs.y, zDepth ), tint, Vec2( uvAtMins.x, uvAtMaxs.y ) ) );
}


//-----------------------------------------------------------------------------------------------
Texture* RenderContext::GetFrameColorTarget()
{
	return m_swapchain->GetBackBuffer();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindShader( Shader* shader )
{
	GUARANTEE_OR_DIE( m_isDrawing, "Tried to call BindShader while not drawing" );

	m_currentShader = shader;
	if ( m_currentShader == nullptr )
	{
		m_currentShader = m_defaultShader;
	}

	m_context->VSSetShader( m_currentShader->m_vertexStage.m_vertexShader, nullptr, 0 );
	m_context->RSSetState( m_currentShader->m_rasterState );
	m_context->PSSetShader( m_currentShader->m_fragmentStage.m_fragmentShader, nullptr, 0 );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindShader( const char* fileName )
{
	Shader* newShader = GetOrCreateShader( fileName );
	BindShader( newShader );
}


//-----------------------------------------------------------------------------------------------
Shader* RenderContext::GetOrCreateShader( const char* filename )
{
	// Check cache for shader
	for ( int loadedShaderIdx = 0; loadedShaderIdx < (int)m_loadedShaders.size(); ++loadedShaderIdx )
	{
		if ( !strcmp( m_loadedShaders[loadedShaderIdx]->GetFileName().c_str(), filename ) )
		{
			return m_loadedShaders[loadedShaderIdx];
		}
	}

	Shader* newShader = new Shader( this );
	newShader->CreateFromFile( filename );
	m_loadedShaders.push_back( newShader );

	return newShader;
}


//-----------------------------------------------------------------------------------------------
Shader* RenderContext::GetOrCreateShaderFromSourceString( const char* shaderName, const char* source )
{
	// Check cache for shader
	for ( int loadedShaderIdx = 0; loadedShaderIdx < (int)m_loadedShaders.size(); ++loadedShaderIdx )
	{
		if ( !strcmp( m_loadedShaders[loadedShaderIdx]->GetFileName().c_str(), shaderName ) )
		{
			return m_loadedShaders[loadedShaderIdx];
		}
	}

	Shader* newShader = new Shader( this );
	newShader->CreateFromSourceString( shaderName, source );
	m_loadedShaders.push_back( newShader );

	return newShader;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindVertexBuffer( VertexBuffer* vbo )
{
	ID3D11Buffer* vboHandle = vbo->m_handle;
	if ( vboHandle == m_lastVBOHandle )
	{
		return;
	}
	m_lastVBOHandle = vboHandle;

	uint stride = (uint)vbo->m_stride;
	uint offset = 0;

	m_context->IASetVertexBuffers( 0, 1, &vboHandle, &stride, &offset );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindIndexBuffer( IndexBuffer* ibo )
{
	ID3D11Buffer* iboHandle = ibo->m_handle;
	if ( iboHandle == m_lastIBOHandle )
	{
		return;
	}
	m_lastIBOHandle = iboHandle;

	m_context->IASetIndexBuffer( iboHandle, DXGI_FORMAT_R32_UINT, 0 );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindUniformBuffer( uint slot, RenderBuffer* ubo )
{
	// TODO: GetHandle
	ID3D11Buffer* uboHandle =  ubo->m_handle;

	m_context->VSSetConstantBuffers( slot, 1, &uboHandle );
	m_context->PSSetConstantBuffers( slot, 1, &uboHandle );
}


//-----------------------------------------------------------------------------------------------
Texture* RenderContext::CreateOrGetTextureFromFile( const char* filePath )
{
	Texture* texture = RetrieveTextureFromCache( filePath );
	if ( texture == nullptr )
	{
		texture = CreateTextureFromFile( filePath );
	}

	return texture;
}


//-----------------------------------------------------------------------------------------------
Texture* RenderContext::CreateTextureFromFile( const char* imageFilePath )
{
	int imageTexelSizeX = 0; // This will be filled in for us to indicate image width
	int imageTexelSizeY = 0; // This will be filled in for us to indicate image height
	int numComponents = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 4; // we support 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* imageData = stbi_load( imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );

	// Check if the load was successful
	if ( imageData == nullptr )
	{
		g_devConsole->PrintString( Stringf( "Failed to load image \"%s\"", imageFilePath ), Rgba8::RED );
		return nullptr;
	}

	if ( !( numComponents == 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0 ) )
	{
		g_devConsole->PrintString( Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, imageTexelSizeX, imageTexelSizeY ) ,Rgba8::RED );
		return nullptr;
	}

	// Describe the texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = imageTexelSizeX;
	desc.Height = imageTexelSizeY;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;						// MSAA
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;				// if we do mip chains this needs to be GPU/DEFAULT; as of now the texture will never change
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	// | RENDER_TAGET later
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = imageData;
	initialData.SysMemPitch = imageTexelSizeX * 4;
	initialData.SysMemSlicePitch = 0;

	// DirectX creation
	ID3D11Texture2D* texHandle = nullptr;
	m_device->CreateTexture2D( &desc, &initialData, &texHandle );

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( imageData );

	Texture* newTexture = new Texture( imageFilePath, this, texHandle );
	m_loadedTextures.push_back( newTexture );

	return newTexture;
}


//-----------------------------------------------------------------------------------------------
Texture* RenderContext::RetrieveTextureFromCache( const char* filePath )
{
	UNUSED( filePath );
	for ( int textureIndex = 0; textureIndex < (int)m_loadedTextures.size(); ++textureIndex )
	{
		if ( m_loadedTextures[textureIndex]->GetFilePath() == filePath )
		{
			return m_loadedTextures[textureIndex];
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::CreateBlendStates()
{
	// Alpha
	D3D11_BLEND_DESC alphaDesc;
	alphaDesc.AlphaToCoverageEnable = FALSE;
	alphaDesc.IndependentBlendEnable = FALSE;
	alphaDesc.RenderTarget[0].BlendEnable = TRUE;
	alphaDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	alphaDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	alphaDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	alphaDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	alphaDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	alphaDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

	alphaDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &alphaDesc, &m_alphaBlendState );

	// Additive
	D3D11_BLEND_DESC additiveDesc;
	additiveDesc.AlphaToCoverageEnable = FALSE;
	additiveDesc.IndependentBlendEnable = FALSE;
	alphaDesc.RenderTarget[0].BlendEnable = TRUE;
	additiveDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	additiveDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

	additiveDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &additiveDesc, &m_additiveBlendState );

	// Opaque
	D3D11_BLEND_DESC opaqueDesc;
	opaqueDesc.AlphaToCoverageEnable = FALSE;
	opaqueDesc.IndependentBlendEnable = FALSE;
	alphaDesc.RenderTarget[0].BlendEnable = FALSE;
	opaqueDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	opaqueDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	opaqueDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;

	opaqueDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	opaqueDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	opaqueDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

	opaqueDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_device->CreateBlendState( &opaqueDesc, &m_disabledBlendState );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindTexture( const Texture* constTexture )
{
	Texture* texture = const_cast<Texture*>( constTexture );
	if ( texture == nullptr )
	{
		texture = m_defaultWhiteTexture;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->m_shaderResourceView;
	m_context->PSSetShaderResources( 0, 1, &srvHandle ); //srv
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindSampler( Sampler* sampler )
{
	if ( sampler == nullptr )
	{
		sampler = m_currentSampler;
	} 

	ID3D11SamplerState* samplerHandle = sampler->m_handle;
	m_context->PSSetSamplers( 0, 1, &samplerHandle );
}


//-----------------------------------------------------------------------------------------------
BitmapFont* RenderContext::CreateOrGetBitmapFontFromFile( const char* filePath )
{
	// Since we have no xml, append .png for now
	std::string fullFilePath( filePath );
	fullFilePath += ".png";

	BitmapFont* font = RetrieveBitmapFontFromCache( fullFilePath.c_str() );

	if ( font == nullptr )
	{
		Texture* fontTexture = CreateOrGetTextureFromFile( fullFilePath.c_str() );

		font = new BitmapFont( fullFilePath.c_str(), fontTexture );
		m_loadedBitmapFonts.push_back( font );
	}

	return font;
}


//-----------------------------------------------------------------------------------------------
Texture* RenderContext::CreateTextureFromColor( const Rgba8& color )
{
	int imageTexelSizeX = 1; // This will be filled in for us to indicate image width
	int imageTexelSizeY = 1; // This will be filled in for us to indicate image height
	//int numComponents = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	//int numComponentsRequested = 4; // we support 4 (32-bit RGBA)

	//// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	//stbi_set_flip_vertically_on_load( 0 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	//unsigned char* initialData = stbi_load_from_memory( &color, 4, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );
	
	// Describe the texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = imageTexelSizeX;
	desc.Height = imageTexelSizeY;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;						// MSAA
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_IMMUTABLE;				// if we do mip chains this needs to be GPU/DEFAULT; as of now the texture will never change
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	// | RENDER_TAGET later
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &color;
	initialData.SysMemPitch = sizeof( Rgba8 );
	initialData.SysMemSlicePitch = 0;

	// DirectX creation
	ID3D11Texture2D* texHandle = nullptr;
	m_device->CreateTexture2D( &desc, &initialData, &texHandle );

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	//stbi_image_free( imageData );

	Texture* newTexture = new Texture( this, texHandle );
	//m_loadedTextures.push_back( newTexture );

	return newTexture;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetModelMatrix( const Mat44& modelMatrix )
{
	ModelMatrixData matrixData;
	matrixData.modelMatrix = modelMatrix;

	m_modelMatrixUBO->Update( &matrixData, sizeof( matrixData ), sizeof( matrixData ) );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::CycleSampler()
{
	if ( m_currentSampler == m_defaultPointSampler )
	{
		m_currentSampler = m_defaultLinearSampler;
	}
	else if ( m_currentSampler == m_defaultLinearSampler )
	{
		m_currentSampler = m_defaultPointSampler;
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::CycleBlendMode()
{
	switch ( m_currentBlendMode )
	{
		case eBlendMode::ALPHA: m_currentBlendMode = eBlendMode::ADDITIVE; return;
		case eBlendMode::ADDITIVE: m_currentBlendMode = eBlendMode::DISABLED; return;
		case eBlendMode::DISABLED: m_currentBlendMode = eBlendMode::ALPHA; return;
	}
}


//-----------------------------------------------------------------------------------------------
BitmapFont* RenderContext::RetrieveBitmapFontFromCache( const char* filePath )
{
	for ( int fontIndex = 0; fontIndex < (int)m_loadedBitmapFonts.size(); ++fontIndex )
	{
		if ( !strcmp( filePath, m_loadedBitmapFonts[fontIndex]->GetFontName().c_str() ) )
		{
			return m_loadedBitmapFonts[fontIndex];
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::CreateDebugModule()
{
	// load the dll
	m_debugModule = ::LoadLibraryA( "Dxgidebug.dll" );
	if ( m_debugModule == nullptr ) 
	{
		g_devConsole->PrintString( "Failed to find dxgidebug.dll.  No debug features enabled.", Rgba8::YELLOW );
	}
	else 
	{
		// find a function in the loaded dll
		typedef HRESULT( WINAPI* GetDebugModuleCB )( REFIID, void** );
		GetDebugModuleCB cb = (GetDebugModuleCB) ::GetProcAddress( (HMODULE)m_debugModule, "DXGIGetDebugInterface" );

		// create our debug object
		HRESULT hr = cb( __uuidof( IDXGIDebug ), (void**)& m_debug );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Failed to create D3D debug object" );
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DestroyDebugModule()
{
	if ( m_debug != nullptr ) 
	{
		DX_SAFE_RELEASE( m_debug );   // release our debug object
		FreeLibrary( (HMODULE)m_debugModule ); // unload the dll

		m_debug = nullptr;
		m_debugModule = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::ReportLiveObjects()
{
	if ( m_debug != nullptr ) 
	{
		HRESULT result = m_debug->ReportLiveObjects( DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL );

		GUARANTEE_OR_DIE( SUCCEEDED( result ), "ReportLiveObjects failed" )
	}
}
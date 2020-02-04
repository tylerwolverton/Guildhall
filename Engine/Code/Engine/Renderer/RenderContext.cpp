#include "RenderContext.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/Polygon2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
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
	m_defaultShader = new Shader( this );
	m_defaultShader->CreateFromFile( "Data/Shaders/Triangle.hlsl" );

	m_immediateVBO = new VertexBuffer( this, MEMORY_HINT_DYNAMIC );
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
	delete m_defaultShader;

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
	UNUSED( blendMode );
	UNIMPLEMENTED();
	/*if ( blendMode == BlendMode::ALPHA )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}
	else if ( blendMode == BlendMode::ADDITIVE )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	}
	else
	{
		ERROR_AND_DIE( Stringf( "Unknown / unsupported blend mode #%i", blendMode ) );
	}*/
}


//-----------------------------------------------------------------------------------------------
void RenderContext::ClearScreen( const Rgba8& clearColor )
{
	float clearFloats[4];
	clearFloats[0] = (float)clearColor.r / 255.f;
	clearFloats[1] = (float)clearColor.g / 255.f;
	clearFloats[2] = (float)clearColor.b / 255.f;
	clearFloats[3] = (float)clearColor.a / 255.f;

	Texture* backbuffer = m_swapchain->GetBackBuffer();
	TextureView* backbuffer_rtv = backbuffer->GetOrCreateRenderTargetView();
	ID3D11RenderTargetView* rtv = backbuffer_rtv->m_renderTargetView;
	m_context->ClearRenderTargetView( rtv, clearFloats );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BeginCamera( const Camera& camera )
{
	if ( camera.GetClearMode() & eCameraClearBitFlag::CLEAR_COLOR_BIT )
	{
		ClearScreen( camera.GetClearColor() );
	}

	BindShader( nullptr );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::EndCamera( const Camera& camera )
{
	UNUSED( camera );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::Draw( int numVertices, int vertexOffset )
{
	Texture* texture = m_swapchain->GetBackBuffer();
	TextureView* view = texture->GetOrCreateRenderTargetView();
	ID3D11RenderTargetView* renderTargetView = view->m_renderTargetView;

	IntVec2 outputSize = texture->GetTexelSize();

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)outputSize.x;
	viewport.Height = (float)outputSize.y;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	// TEMPORARY - move this later
	m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	m_context->VSSetShader( m_currentShader->m_vertexStage.m_vertexShader, nullptr, 0 );
	m_context->RSSetState( m_currentShader->m_rasterState );
	m_context->RSSetViewports( 1, &viewport );
	m_context->PSSetShader( m_currentShader->m_fragmentStage.m_fragmentShader, nullptr, 0 );
	m_context->OMSetRenderTargets( 1, &renderTargetView, nullptr );

	m_context->Draw( numVertices, vertexOffset );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( int numVertices, const Vertex_PCU* vertices )
{
	// RenderBuffer* m_immediateVBO;  // VBO - vertex buffer object

	// Update a vertex buffer
	size_t dataByteSize = numVertices * sizeof( Vertex_PCU );
	size_t elementSize = sizeof( Vertex_PCU );
	m_immediateVBO->Update( vertices, dataByteSize, elementSize );

	// Bind
	BindVertexInput( m_immediateVBO );

	// Draw
	Draw( numVertices, 0 );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( const std::vector<Vertex_PCU>& vertices )
{
	DrawVertexArray( (int)vertices.size(), &vertices[0] );
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
void RenderContext::DrawOBB2( const OBB2& box, const Rgba8& tint )
{
	std::vector<Vertex_PCU> vertices;

	AppendVertsForOBB2D( vertices, box, tint );

	DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
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
		g_devConsole->PrintString( Rgba8::YELLOW, Stringf( "Tried to draw a Polygon2Outline with %d vertices. At least 3 vertices are required.", numVertices ));
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
		g_devConsole->PrintString( Rgba8::YELLOW, Stringf( "Tried to append verts for a Polygon2 with %d vertices. At least 3 vertices are required.", numVertices ) );
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
void RenderContext::BindShader( Shader* shader )
{
	m_currentShader = shader;
	if ( m_currentShader == nullptr )
	{
		m_currentShader = m_defaultShader;
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindVertexInput( VertexBuffer* vbo )
{
	ID3D11Buffer* vboHandle = vbo->m_handle;
	uint stride = sizeof( Vertex_PCU );
	uint offset = 0;

	m_context->IASetVertexBuffers( 0, 1, &vboHandle, &stride, &offset );
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
	UNUSED( imageFilePath );
	UNIMPLEMENTED();
	return nullptr;
	//unsigned int textureID = 0;
	//int imageTexelSizeX = 0; // This will be filled in for us to indicate image width
	//int imageTexelSizeY = 0; // This will be filled in for us to indicate image height
	//int numComponents = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	//int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	//// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	//stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	//unsigned char* imageData = stbi_load( imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );

	//// Check if the load was successful
	//if ( imageData == nullptr )
	//{
	//	g_devConsole->PrintString( Rgba8::RED, Stringf( "Failed to load image \"%s\"", imageFilePath ) );
	//	return nullptr;
	//}

	//if ( !( numComponents >= 3 && numComponents <= 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0 ) )
	//{
	//	g_devConsole->PrintString( Rgba8::RED, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, imageTexelSizeX, imageTexelSizeY ) );
	//	return nullptr;
	//}

	//// Enable OpenGL texturing
	//glEnable( GL_TEXTURE_2D );

	//// Tell OpenGL that our pixel data is single-byte aligned
	//glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	//// Ask OpenGL for an unused texName (ID number) to use for this texture
	//glGenTextures( 1, (GLuint*)& textureID );

	//// Tell OpenGL to bind (set) this as the currently active texture
	//glBindTexture( GL_TEXTURE_2D, textureID );

	//// Set texture clamp vs. wrap (repeat) default settings
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // GL_CLAMP or GL_REPEAT
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // GL_CLAMP or GL_REPEAT

	//// Set magnification (texel > pixel) and minification (texel < pixel) filters
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	//// Pick the appropriate OpenGL format (RGB or RGBA) for this texel data
	//GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	//if ( numComponents == 3 )
	//{
	//	bufferFormat = GL_RGB;
	//}
	//GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; technically allows us to translate into a different texture format as we upload to OpenGL

	//// Upload the image texel data (raw pixels bytes) to OpenGL under this textureID
	//glTexImage2D(			// Upload this pixel data to our new OpenGL texture
	//			  GL_TEXTURE_2D,		// Creating this as a 2d texture
	//			  0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
	//			  internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
	//			  imageTexelSizeX,	// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
	//			  imageTexelSizeY,	// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
	//			  0,					// Border size, in texels (must be 0 or 1, recommend 0)
	//			  bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
	//			  GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
	//			  imageData );		// Address of the actual pixel data bytes/buffer in system memory

	//		  // Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	//stbi_image_free( imageData );

	//Texture* newTexture = new Texture( textureID, imageFilePath, IntVec2( imageTexelSizeX, imageTexelSizeY ) );
	//m_loadedTextures.push_back( newTexture );

	//return newTexture ;
}


//-----------------------------------------------------------------------------------------------
Texture* RenderContext::RetrieveTextureFromCache( const char* filePath )
{
	UNUSED( filePath );
	/*for ( int textureIndex = 0; textureIndex < (int)m_loadedTextures.size(); ++textureIndex )
	{
		if ( !strcmp( filePath, m_loadedTextures[textureIndex]->GetFilePath() ) )
		{
			return m_loadedTextures[textureIndex];
		}
	}*/

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindTexture( const Texture* texture )
{
	UNUSED( texture );
	UNIMPLEMENTED();
	/*if ( texture )
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, texture->GetTextureID() );
	}
	else
	{
		glDisable( GL_TEXTURE_2D );
	}*/
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
		g_devConsole->PrintString( Rgba8::YELLOW, "Failed to find dxgidebug.dll.  No debug features enabled." );
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
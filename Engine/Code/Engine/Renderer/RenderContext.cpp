#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/BuiltInShaders.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"

#include "ThirdParty/stb/stb_image.h"


//-----------------------------------------------------------------------------------------------
// DX3D11 Includes
//-----------------------------------------------------------------------------------------------
#pragma comment( lib, "d3d11.lib" )         // needed a01
#pragma comment( lib, "dxgi.lib" )          // needed a01
#pragma comment( lib, "d3dcompiler.lib" )   // needed when we get to shaders


//-----------------------------------------------------------------------------------------------
void RenderContext::Startup( Window* window )
{
	InitializeSwapChain( window );
	InitializeDefaultRenderObjects();

	m_systemFont = CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );

	GUARANTEE_OR_DIE( m_systemFont != nullptr, "Could not load default system font. Make sure it is in the Data/Fonts directory." );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::Setup( Clock* gameClock )
{
	m_gameClock = gameClock;
	if ( m_gameClock == nullptr )
	{
		m_gameClock = Clock::GetMaster();
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{
	UpdateFrameData();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	m_swapchain->Present();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::Shutdown()
{
	PTR_SAFE_DELETE( m_immediateVBO );
	PTR_SAFE_DELETE( m_immediateIBO );
	PTR_SAFE_DELETE( m_frameUBO );
	PTR_SAFE_DELETE( m_modelMatrixUBO );
	PTR_SAFE_DELETE( m_materialUBO );
	PTR_SAFE_DELETE( m_lightUBO );
		
	PTR_VECTOR_SAFE_DELETE( m_loadedBitmapFonts );
	PTR_VECTOR_SAFE_DELETE( m_loadedTextures );
	PTR_VECTOR_SAFE_DELETE( m_loadedShaders );
	PTR_VECTOR_SAFE_DELETE( m_loadedShaderPrograms );
	PTR_VECTOR_SAFE_DELETE( m_loadedSamplers );

	PTR_SAFE_DELETE( m_swapchain );

	DX_SAFE_RELEASE( m_defaultRasterState );
	DX_SAFE_RELEASE( m_currentRasterState );
	DX_SAFE_RELEASE( m_alphaBlendState );
	DX_SAFE_RELEASE( m_additiveBlendState );
	DX_SAFE_RELEASE( m_disabledBlendState );
	DX_SAFE_RELEASE( m_currentDepthStencilState );
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
void RenderContext::SetDepthTest( eCompareFunc compare, bool writeDepthOnPass )
{
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = writeDepthOnPass;
	desc.DepthFunc = ToDxComparisonFunc( compare );
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.StencilEnable = false;

	DX_SAFE_RELEASE( m_currentDepthStencilState );

	m_device->CreateDepthStencilState( &desc, &m_currentDepthStencilState );
	m_context->OMSetDepthStencilState( m_currentDepthStencilState, 0 );
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
void RenderContext::ClearDepth( Texture* depthStencilTarget, float depth )
{
	TextureView* view = depthStencilTarget->GetOrCreateDepthStencilView();
	ID3D11DepthStencilView* dsv = view->m_depthStencilView;
	m_context->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH, depth, 0 );
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
	
	TextureView* view = colorTarget->GetOrCreateRenderTargetView();
	ID3D11RenderTargetView* renderTargetView = view->m_renderTargetView;

	UpdateAndBindBuffers( camera );
	SetupRenderTargetViewWithDepth( renderTargetView, camera );
	InitializeViewport( colorTarget->GetTexelSize() );
	
	ClearCamera( renderTargetView, camera );
	ResetRenderObjects();

	SetModelData( Mat44::IDENTITY );
	SetBlendMode( m_currentBlendMode );
	SetAmbientLight( Rgba8::WHITE, 1.f );
	// Dirty all state, booleans or a uint flags
}


//-----------------------------------------------------------------------------------------------
void RenderContext::EndCamera( const Camera& camera )
{
	UNUSED( camera );
	DX_SAFE_RELEASE( m_currentDepthStencilState );
	m_isDrawing = false;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::UpdateFrameData()
{
	FrameData frameData;
	frameData.systemTimeSeconds = (float)Clock::GetMaster()->GetTotalElapsedSeconds();
	frameData.systemDeltaTimeSeconds = (float)m_gameClock->GetLastDeltaSeconds();

	frameData.nearFogDistance = m_linearFog.nearFogDistance;
	frameData.farFogDistance = m_linearFog.farFogDistance;
	frameData.fogColor = m_linearFog.fogColor;

	frameData.gamma = m_gamma;

	m_frameUBO->Update( &frameData, sizeof( frameData ), sizeof( frameData ) );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::Draw( int numVertices, int vertexOffset )
{
	FinalizeContext();

	m_context->Draw( numVertices, vertexOffset );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DrawIndexed( int indexCount, int indexOffset, int vertexOffset )
{
	FinalizeContext();

	m_context->DrawIndexed( indexCount, indexOffset, vertexOffset );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::FinalizeContext()
{
	GUARANTEE_OR_DIE( m_lastBoundVBO != nullptr, "No vbo bound before draw call" );

	// Describe Vertex Format to Shader
	ID3D11InputLayout* inputLayout = m_currentShaderProgram->GetOrCreateInputLayout( m_lastBoundVBO->m_attributes );
	m_context->IASetInputLayout( inputLayout );

	SetLightData();

	BindUniformBuffer( UBO_LIGHT_SLOT, m_lightUBO );
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
void RenderContext::DrawMesh( GPUMesh* mesh )
{	
	// Bind
	BindVertexBuffer( mesh->m_vertices );
	//BindVertexBuffer( mesh->m_vertices, mesh->GetVertexLayout() );
	
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
Texture* RenderContext::GetBackBuffer()
{
	return m_swapchain->GetBackBuffer();
}


//-----------------------------------------------------------------------------------------------
IntVec2 RenderContext::GetDefaultBackBufferSize()
{
	return m_swapchain->GetBackBuffer()->GetTexelSize();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindShaderProgram( ShaderProgram* shader )
{
	GUARANTEE_OR_DIE( m_isDrawing, "Tried to call BindShader while not drawing" );

	m_currentShaderProgram = shader;
	if ( m_currentShaderProgram == nullptr )
	{
		m_currentShaderProgram = m_defaultShaderProgram;
	}

	m_context->VSSetShader( m_currentShaderProgram->m_vertexStage.m_vertexShader, nullptr, 0 );
	m_context->PSSetShader( m_currentShaderProgram->m_fragmentStage.m_fragmentShader, nullptr, 0 );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindShaderProgram( const char* fileName )
{
	ShaderProgram* newShader = GetOrCreateShaderProgram( fileName );
	BindShaderProgram( newShader );
}


//-----------------------------------------------------------------------------------------------
Shader* RenderContext::GetOrCreateShader( const char* filename )
{
	// Check cache for shader
	for ( int loadedShaderIdx = 0; loadedShaderIdx < (int)m_loadedShaders.size(); ++loadedShaderIdx )
	{
		if ( !strcmp( m_loadedShaders[loadedShaderIdx]->GetFileName(), filename ) )
		{
			return m_loadedShaders[loadedShaderIdx];
		}
	}

	Shader* newShader = new Shader( this, filename );
	m_loadedShaders.push_back( newShader );

	return newShader;
}


//-----------------------------------------------------------------------------------------------
ShaderProgram* RenderContext::GetOrCreateShaderProgram( const char* filename )
{
	// Check cache for shader program
	for ( int loadedShaderProgramIdx = 0; loadedShaderProgramIdx < (int)m_loadedShaderPrograms.size(); ++loadedShaderProgramIdx )
	{
		if ( !strcmp( m_loadedShaderPrograms[loadedShaderProgramIdx]->GetFileName().c_str(), filename ) )
		{
			return m_loadedShaderPrograms[loadedShaderProgramIdx];
		}
	}

	ShaderProgram* newShaderProgram = new ShaderProgram( this );
	newShaderProgram->CreateFromFile( filename );
	m_loadedShaderPrograms.push_back( newShaderProgram );

	return newShaderProgram;
}


//-----------------------------------------------------------------------------------------------
ShaderProgram* RenderContext::GetOrCreateShaderProgramFromSourceString( const char* shaderName, const char* source )
{
	// Check cache for shader
	for ( int loadedShaderProgramIdx = 0; loadedShaderProgramIdx < (int)m_loadedShaderPrograms.size(); ++loadedShaderProgramIdx )
	{
		if ( !strcmp( m_loadedShaderPrograms[loadedShaderProgramIdx]->GetFileName().c_str(), shaderName ) )
		{
			return m_loadedShaderPrograms[loadedShaderProgramIdx];
		}
	}

	ShaderProgram* newShaderProgram = new ShaderProgram( this );
	newShaderProgram->CreateFromSourceString( shaderName, source );
	m_loadedShaderPrograms.push_back( newShaderProgram );

	return newShaderProgram;
}


//-----------------------------------------------------------------------------------------------
Shader* RenderContext::GetShaderByName( std::string shaderName )
{
	for ( int loadedShaderIdx = 0; loadedShaderIdx < (int)m_loadedShaders.size(); ++loadedShaderIdx )
	{
		if ( m_loadedShaders[loadedShaderIdx]->GetName() == shaderName )
		{
			return m_loadedShaders[loadedShaderIdx];
		}
	}

	//ERROR_AND_DIE( Stringf( "No shader named '%s' has been loaded.", shaderName.c_str() ) );
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::ReloadShaders()
{
	for ( int shaderIdx = 0; shaderIdx < (int)m_loadedShaderPrograms.size(); ++shaderIdx )
	{
		ShaderProgram*& shader = m_loadedShaderPrograms[shaderIdx];
		if ( shader != nullptr )
		{
			shader->ReloadFromDisc();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindVertexBuffer( VertexBuffer* vbo )
{
	ID3D11Buffer* vboHandle = vbo->m_handle;
	m_lastBoundVBO = vbo;
	if ( vboHandle == m_lastVBOHandle )
	{
		return;
	}
	m_lastVBOHandle = vboHandle;

	// Finalize state can check this bool || if shader has changed before setting input layout
	// if(m_lastBoundLayout != vb0->GetLayout)
	// m_layoutHasChanged=  true



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
	ID3D11Buffer* uboHandle =  ubo->m_handle;

	m_context->VSSetConstantBuffers( slot, 1, &uboHandle );
	m_context->PSSetConstantBuffers( slot, 1, &uboHandle );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindMaterial( Material* material )
{
	BindShader( material->m_shader );

	BindDiffuseTexture( material->m_diffuseTexture );
	BindNormalTexture( material->m_normalTexture );

	for( uint textureIdx = 0; textureIdx < material->m_userTextures.size(); ++textureIdx )
	{
		Texture*& texture = material->m_userTextures[textureIdx];
		if ( texture != nullptr )
		{
			BindTexture( USER_TEXTURE_SLOT_START + textureIdx, texture );
		}
	}

	for ( uint samplerIdx = 0; samplerIdx < material->m_userTextures.size(); ++samplerIdx )
	{
		Sampler*& sampler = material->m_userSamplers[samplerIdx];
		if ( sampler != nullptr )
		{
			BindSampler( USER_TEXTURE_SLOT_START + samplerIdx, sampler );
		}
	}

	material->UpdateUBOIfDirty();
	BindUniformBuffer( UBO_MATERIAL_SLOT, material->m_ubo );

	SetModelData( m_modelMatrix, material->m_tint, material->m_specularFactor, material->m_specularPower );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindShader( Shader* shader )
{
	BindShaderProgram( shader->GetShaderProgram() );
	
	ShaderState state = shader->GetShaderState();
	SetCullMode( state.cullMode );
	SetFillMode( state.fillMode );
	SetFrontFaceWindOrder( state.isWindingCCW );

	SetBlendMode( state.blendMode );

	SetDepthTest( state.depthTestCompare, state.writeDpeth );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindShaderByName( std::string shaderName )
{
	for ( int loadedShaderIdx = 0; loadedShaderIdx < (int)m_loadedShaders.size(); ++loadedShaderIdx )
	{
		if ( m_loadedShaders[loadedShaderIdx]->GetName() == shaderName )
		{
			BindShader( m_loadedShaders[loadedShaderIdx] );
			return;
		}
	}

	ERROR_AND_DIE( Stringf( "No shader named '%s' has been loaded.", shaderName.c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindShaderByPath( const char* filePath )
{
	Shader* shader = GetOrCreateShader( filePath );
	if ( shader == nullptr )
	{
		// Error shader
	}

	BindShader( shader );
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
void RenderContext::InitializeSwapChain( Window* window )
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
}


//-----------------------------------------------------------------------------------------------
void RenderContext::InitializeDefaultRenderObjects()
{
	// Create default shader
	m_defaultShaderProgram = GetOrCreateShaderProgramFromSourceString( "DefaultBuiltInShader", g_defaultShaderCode );

	// Create default buffers
	m_immediateVBO = new VertexBuffer( this, MEMORY_HINT_DYNAMIC, sizeof( Vertex_PCU ), Vertex_PCU::LAYOUT );
	m_immediateIBO = new IndexBuffer( this, MEMORY_HINT_DYNAMIC );
	m_frameUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_modelMatrixUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_materialUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	m_lightUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );

	// Create default sampler
	m_defaultSampler = GetOrCreateSampler( SAMPLER_POINT, UV_MODE_CLAMP );

	// Create a white texture to use when no texture is needed
	m_defaultWhiteTexture = CreateTextureFromColor( Rgba8::WHITE );
	m_flatNormalMap = CreateTextureFromColor( Rgba8( 127, 127, 255, 255 ) );

	// Create a depth buffer and initialize it to draw pixels using painter's algorithm
	m_defaultDepthBuffer = GetOrCreateDepthStencil( m_swapchain->GetBackBuffer()->GetTexelSize() );
	SetDepthTest( eCompareFunc::COMPARISON_ALWAYS, false );

	CreateDefaultRasterState();
	SetRasterState( eFillMode::SOLID, eCullMode::BACK, true );

	CreateBlendStates();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::InitializeViewport( const IntVec2& outputSize )
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)outputSize.x;
	viewport.Height = (float)outputSize.y;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	m_context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_context->RSSetViewports( 1, &viewport );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::UpdateAndBindBuffers( Camera& camera )
{
	if ( camera.m_cameraUBO == nullptr )
	{
		camera.m_cameraUBO = new RenderBuffer( this, UNIFORM_BUFFER_BIT, MEMORY_HINT_DYNAMIC );
	}
	camera.UpdateCameraUBO();

	BindUniformBuffer( UBO_FRAME_SLOT, m_frameUBO );
	BindUniformBuffer( UBO_CAMERA_SLOT, camera.m_cameraUBO );
	BindUniformBuffer( UBO_MODEL_MATRIX_SLOT, m_modelMatrixUBO );
	BindUniformBuffer( UBO_LIGHT_SLOT, m_lightUBO );
	BindUniformBuffer( UBO_MATERIAL_SLOT, m_materialUBO );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetupRenderTargetViewWithDepth( ID3D11RenderTargetView* renderTargetView, const Camera& camera )
{
	Texture* depthStencilTarget = camera.GetDepthStencilTarget();
	TextureView* depthView = nullptr;
	if ( depthStencilTarget != nullptr )
	{
		depthView = depthStencilTarget->GetOrCreateDepthStencilView();
	}

	ID3D11DepthStencilView* depthStencilView = nullptr;
	if ( depthView != nullptr )
	{
		depthStencilView = depthView->m_depthStencilView;
	}

	m_context->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::ClearCamera( ID3D11RenderTargetView* renderTargetView, const Camera& camera )
{
	if ( camera.GetClearMode() & eCameraClearBitFlag::CLEAR_COLOR_BIT )
	{
		ClearScreen( renderTargetView, camera.GetClearColor() );
	}

	if ( camera.GetClearMode() & eCameraClearBitFlag::CLEAR_DEPTH_BIT
		 && camera.GetDepthStencilTarget() != nullptr )
	{
		ClearDepth( camera.GetDepthStencilTarget(), camera.GetClearDepth() );
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::ResetRenderObjects()
{
	BindShaderProgram( ( ShaderProgram* )nullptr );
	BindDiffuseTexture( nullptr );
	BindNormalTexture( nullptr );
	BindTexture( 8, nullptr );
	BindSampler( 0, nullptr );
	m_context->RSSetState( m_defaultRasterState );
	m_lastVBOHandle = nullptr;
	m_lastIBOHandle = nullptr;
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
void RenderContext::SetRasterState( eFillMode fillMode, eCullMode cullMode, bool windCCW )
{
	D3D11_RASTERIZER_DESC desc;

	desc.FillMode = ToDXFillMode( fillMode );
	desc.CullMode = ToDXCullMode( cullMode );
	desc.FrontCounterClockwise = windCCW;
	desc.DepthBias = 0U;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = TRUE;
	desc.ScissorEnable = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;
		
	DX_SAFE_RELEASE( m_currentRasterState );

	m_device->CreateRasterizerState( &desc, &m_currentRasterState );
	m_context->RSSetState( m_currentRasterState );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::CreateDefaultRasterState()
{
	D3D11_RASTERIZER_DESC desc;

	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_BACK;
	desc.FrontCounterClockwise = true;
	desc.DepthBias = 0U;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = TRUE;
	desc.ScissorEnable = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;
		
	m_device->CreateRasterizerState( &desc, &m_defaultRasterState );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindDiffuseTexture( const Texture* constTexture )
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
void RenderContext::BindNormalTexture( const Texture* constTexture )
{
	Texture* texture = const_cast<Texture*>( constTexture );
	if ( texture == nullptr )
	{
		texture = m_flatNormalMap;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->m_shaderResourceView;
	m_context->PSSetShaderResources( 1, 1, &srvHandle ); //srv
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindTexture( uint slot, const Texture* constTexture )
{
	Texture* texture = const_cast<Texture*>( constTexture );
	if ( texture == nullptr )
	{
		// TODO: Make error texture
		texture = m_defaultWhiteTexture;
	}

	TextureView* shaderResourceView = texture->GetOrCreateShaderResourceView();
	ID3D11ShaderResourceView* srvHandle = shaderResourceView->m_shaderResourceView;
	m_context->PSSetShaderResources( slot, 1, &srvHandle ); //srv
}


//-----------------------------------------------------------------------------------------------
void RenderContext::BindSampler( uint slot, Sampler* sampler )
{
	if ( sampler == nullptr )
	{
		sampler = m_defaultSampler;
	} 

	ID3D11SamplerState* samplerHandle = sampler->m_handle;
	m_context->PSSetSamplers( slot, 1, &samplerHandle );
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
Sampler* RenderContext::GetOrCreateSampler( eSamplerType filter, eSamplerUVMode mode )
{
	for ( int samplerIndex = 0; samplerIndex < (int)m_loadedSamplers.size(); ++samplerIndex )
	{
		if ( m_loadedSamplers[samplerIndex]->m_filter == filter
			 && m_loadedSamplers[samplerIndex]->m_mode == mode )
		{
			return m_loadedSamplers[samplerIndex];
		}
	}

	Sampler* newSampler = new Sampler( this, filter, mode );
	m_loadedSamplers.push_back( newSampler );
	
	return newSampler;
}


//-----------------------------------------------------------------------------------------------
Texture* RenderContext::CreateTextureFromColor( const Rgba8& color )
{
	int imageTexelSizeX = 1; 
	int imageTexelSizeY = 1; 

	// Describe the texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = imageTexelSizeX;
	desc.Height = imageTexelSizeY;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;						// MSAA
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;				// if we do mip chains this needs to be GPU/DEFAULT; as of now the texture will never change
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	
	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &color;
	initialData.SysMemPitch = sizeof( Rgba8 );
	initialData.SysMemSlicePitch = 0;

	// DirectX creation
	ID3D11Texture2D* texHandle = nullptr;
	m_device->CreateTexture2D( &desc, &initialData, &texHandle );
	
	Texture* newTexture = new Texture( this, texHandle );
	m_loadedTextures.push_back( newTexture );

	return newTexture;
}


//-----------------------------------------------------------------------------------------------
Texture* RenderContext::GetOrCreateDepthStencil( const IntVec2& outputDimensions )
{
	// Find depth stencil in cache?

	// Describe the texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = outputDimensions.x;
	desc.Height = outputDimensions.y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;						// MSAA
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;		
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	// use BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	
	// DirectX creation
	ID3D11Texture2D* texHandle = nullptr;
	m_device->CreateTexture2D( &desc, nullptr, &texHandle );
	
	Texture* newTexture = new Texture( this, texHandle );
	m_loadedTextures.push_back( newTexture );

	return newTexture;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetModelMatrix( const Mat44& modelMatrix )
{
	m_modelMatrix = modelMatrix;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetModelData( const Mat44& modelMatrix, const Rgba8& tint, float specularFactor, float specularPower )
{
	ModelData matrixData;
	matrixData.modelMatrix = modelMatrix;
	tint.GetAsFloatArray( matrixData.tint );

	matrixData.specularFactor = specularFactor;
	matrixData.specularPower = specularPower;

	m_modelMatrixUBO->Update( &matrixData, sizeof( matrixData ), sizeof( matrixData ) );

	m_modelMatrix = modelMatrix;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetMaterialData( void* materialData, int dataSize )
{
	m_materialUBO->Update( materialData, dataSize, dataSize );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetLightData()
{
	LightData lightData;
	lightData.ambientLight = Vec4( m_ambientLightColor, m_ambientLightIntensity );
	memcpy( lightData.lights, m_lights, sizeof( m_lights ) );

	m_lightUBO->Update( &lightData, sizeof( lightData ), sizeof( lightData ) );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetCullMode( eCullMode cullMode )
{
	D3D11_RASTERIZER_DESC desc;
	m_currentRasterState->GetDesc( &desc );

	// Don't do anything if the raster state already matches the requested change
	if ( ToDXCullMode( cullMode ) == desc.CullMode )
	{
		return;
	}

	SetRasterState( FromDXFillMode( desc.FillMode ), cullMode, desc.FrontCounterClockwise );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetFillMode( eFillMode fillMode )
{
	D3D11_RASTERIZER_DESC desc;
	m_currentRasterState->GetDesc( &desc );

	// Don't do anything if the raster state already matches the requested change
	if ( ToDXFillMode( fillMode ) == desc.FillMode )
	{
		return;
	}
	
	SetRasterState( fillMode, FromDXCullMode( desc.CullMode ), desc.FrontCounterClockwise );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetFrontFaceWindOrder( bool windCCW )
{
	D3D11_RASTERIZER_DESC desc;
	m_currentRasterState->GetDesc( &desc );

	// Don't do anything if the raster state already matches the requested change
	if ( (BOOL)windCCW == desc.FrontCounterClockwise )
	{
		return;
	}

	SetRasterState( FromDXFillMode( desc.FillMode ), FromDXCullMode( desc.CullMode ), windCCW );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetAmbientColor( const Rgba8& color )
{
	float vals[4];
	color.GetAsFloatArray( vals );

	m_ambientLightColor = Vec3( vals[0], vals[1], vals[2] );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetAmbientColor( const Vec3& color )
{
	m_ambientLightColor = color;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetAmbientIntensity( float intensity )
{
	m_ambientLightIntensity = intensity;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetAmbientLight( const Rgba8& color, float intensity )
{
	SetAmbientColor( color );
	SetAmbientIntensity( intensity );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetAmbientLight( const Vec3& color, float intensity )
{
	SetAmbientColor( color );
	SetAmbientIntensity( intensity );
}


//-----------------------------------------------------------------------------------------------
void RenderContext::EnableLight( uint idx, const Light& lightInfo )
{
	m_lights[idx] = lightInfo;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DisableLight( uint idx )
{
	m_lights[idx] = Light();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DisableAllLights()
{
	for ( int lightNum = 0; lightNum < MAX_LIGHTS; ++lightNum )
	{
		DisableLight( lightNum );
	}
}


//-----------------------------------------------------------------------------------------------
void RenderContext::SetGamma( float gamma )
{
	m_gamma = gamma;
}


//-----------------------------------------------------------------------------------------------
void RenderContext::EnableFog( float nearFogDist, float farFogDist, const Rgba8& fogColor )
{
	m_linearFog.nearFogDistance = nearFogDist;
	m_linearFog.farFogDistance = farFogDist;
	m_linearFog.fogColor = fogColor.GetAsRGBAVector();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::DisableFog()
{
	m_linearFog.nearFogDistance = 99999.f;
	m_linearFog.farFogDistance = 99999.f;
	m_linearFog.fogColor = Rgba8::BLACK.GetAsRGBAVector();
}


//-----------------------------------------------------------------------------------------------
void RenderContext::CycleBlendMode()
{
	switch ( m_currentBlendMode )
	{
		case eBlendMode::ALPHA:		m_currentBlendMode = eBlendMode::ADDITIVE; return;
		case eBlendMode::ADDITIVE:	m_currentBlendMode = eBlendMode::DISABLED; return;
		case eBlendMode::DISABLED:	m_currentBlendMode = eBlendMode::ALPHA; return;
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

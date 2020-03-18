#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct IDXGIDebug;
struct ID3D11RenderTargetView;
struct ID3D11Buffer;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct Rgba8;
struct Vertex_PCU;
class Window;
class Clock;
class Polygon2;
class Camera;
class Texture;
class Sampler;
class BitmapFont;
class Shader;
class RenderBuffer;
class SwapChain;
class VertexBuffer;
class IndexBuffer;
class GPUMesh;
enum class eCompareFunc : uint;


//-----------------------------------------------------------------------------------------------
enum class eBlendMode
{
	DISABLED,
	ALPHA,
	ADDITIVE,
};


//-----------------------------------------------------------------------------------------------
enum eBufferSlot
{
	UBO_FRAME_SLOT = 0,
	UBO_CAMERA_SLOT = 1,
	UBO_MODEL_MATRIX_SLOT = 2,
};


//-----------------------------------------------------------------------------------------------
struct FrameData
{
	float systemTimeSeconds;
	float systemDeltaTimeSeconds;
	
	float padding[2];
};


//-----------------------------------------------------------------------------------------------
struct ModelMatrixData
{
	Mat44 modelMatrix;
};


//-----------------------------------------------------------------------------------------------
class RenderContext
{
public:
	void Startup( Window* window );
	void Setup( Clock* gameClock );
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void SetBlendMode( eBlendMode blendMode );
	void SetDepthTest( eCompareFunc compare, bool writeDepthOnPass );

	void ClearScreen( ID3D11RenderTargetView* renderTargetView, const Rgba8& clearColor );
	void ClearDepth( Texture* depthStencilTarget, float depth );

	void BeginCamera( Camera& camera );
	void EndCamera	( const Camera& camera );

	void UpdateFrameTime();

	void Draw( int numVertices, int vertexOffset = 0 );
	void DrawIndexed( int indexCount, int indexOffset = 0, int vertexOffset = 0 );

	void DrawVertexArray( int numVertices, const Vertex_PCU* vertices );
	void DrawVertexArray( const std::vector<Vertex_PCU>& vertices );
	void DrawMesh( GPUMesh* mesh );
	
	Texture* GetFrameColorTarget();
	IntVec2 GetDefaultBackBufferSize();

	// Binding Inputs
	void BindVertexBuffer( VertexBuffer* vbo );
	void BindIndexBuffer( IndexBuffer* ibo );
	void BindUniformBuffer( uint slot, RenderBuffer* ubo );

	// Binding State
	void BindShader( Shader* shader );
	void BindShader( const char* fileName );
	void BindTexture( const Texture* constTexture );
	void BindSampler( Sampler* sampler );

	// Resource Creation
	Shader* GetOrCreateShader( const char* filename );
	Shader* GetOrCreateShaderFromSourceString( const char* shaderName, const char* source );
	Texture* CreateOrGetTextureFromFile( const char* filePath );
	Texture* CreateTextureFromColor( const Rgba8& color );
	Texture* GetOrCreateDepthStencil( const IntVec2& outputDimensions );
	BitmapFont* CreateOrGetBitmapFontFromFile( const char* filePath );

	//Texture* CreateTextureFromImage( ... ); for cleaning up D3D calls

	void SetModelMatrix( const Mat44& modelMatrix );

	// Debug methods
	void CycleSampler();
	void CycleBlendMode();

private:
	void InitializeSwapChain( Window* window );
	void InitializeDefaultRenderObjects();
	void InitializeViewport( const IntVec2& outputSize );
	void UpdateAndBindBuffers( Camera& camera );
	void SetupRenderTargetViewWithDepth( ID3D11RenderTargetView* renderTargetView, const Camera& camera );
	void ClearCamera( ID3D11RenderTargetView* renderTargetView, const Camera& camera );
	void ResetRenderObjects();

	Texture* CreateTextureFromFile( const char* filePath );
	Texture* RetrieveTextureFromCache( const char* filePath );

	void CreateBlendStates();

	ID3D11RasterizerState* CreateRasterState();

	BitmapFont* RetrieveBitmapFontFromCache( const char* filePath );

	void CreateDebugModule();
	void DestroyDebugModule();
	void ReportLiveObjects();

public:
	// SD2 TODO: Move to D3D11Common.hpp
	ID3D11Device* m_device			= nullptr;
	ID3D11DeviceContext* m_context	= nullptr;		// immediate context
	SwapChain* m_swapchain			= nullptr;		// gives us textures that we can draw that the user can see

	void* m_debugModule				= nullptr;
	IDXGIDebug* m_debug				= nullptr;

	VertexBuffer* m_immediateVBO	= nullptr;
	IndexBuffer* m_immediateIBO		= nullptr;
	RenderBuffer* m_frameUBO		= nullptr;
	RenderBuffer* m_modelMatrixUBO	= nullptr;

private:
	Clock* m_gameClock								= nullptr;

	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont*> m_loadedBitmapFonts;

	ID3D11Buffer* m_lastVBOHandle					= nullptr;
	ID3D11Buffer* m_lastIBOHandle					= nullptr;

	Shader* m_defaultShader							= nullptr;
	Shader* m_currentShader							= nullptr;
	std::vector<Shader*> m_loadedShaders;

	Sampler* m_defaultPointSampler					= nullptr;
	Sampler* m_defaultLinearSampler					= nullptr;
	Sampler* m_currentSampler						= nullptr;
	
	Texture* m_defaultWhiteTexture					= nullptr;
	Texture* m_defaultDepthBuffer					= nullptr;

	ID3D11DepthStencilState* m_currentDepthStencilState = nullptr;

	ID3D11BlendState* m_alphaBlendState				= nullptr;
	ID3D11BlendState* m_additiveBlendState			= nullptr;
	ID3D11BlendState* m_disabledBlendState			= nullptr;
	eBlendMode m_currentBlendMode					= eBlendMode::ALPHA;

	ID3D11RasterizerState* m_defaultRasterState		= nullptr;
	ID3D11RasterizerState* m_currentRasterState		= nullptr;

	bool m_isDrawing								= false;
};

#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexFont.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
constexpr int USER_TEXTURE_SLOT_START = 8;
constexpr int MAX_USER_TEXTURES = 8;


//-----------------------------------------------------------------------------------------------
//
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct IDXGIDebug;
struct ID3D11RenderTargetView;
struct ID3D11Buffer;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilView;
struct Vertex_PCU;
struct Vertex_PCUTBN;
class BitmapFont;
class Camera;
class Clock;
class GPUMesh;
class IndexBuffer;
class Material;
class Polygon2;
class RenderBuffer;
class Sampler;
class Shader;
class ShaderProgram;
class SwapChain;
class Texture;
class Window;
enum class eCompareFunc : uint;
enum class eFillMode : uint;
enum class eCullMode : uint;
enum eSamplerType : uint;
enum eSamplerUVMode : uint;

//-----------------------------------------------------------------------------------------------
class RenderContext
{
public:
	virtual void Startup( Window* window );
	virtual void Setup( Clock* gameClock );
	virtual void BeginFrame();
	virtual void EndFrame();
	virtual void Shutdown();

	void SetBlendMode( eBlendMode blendMode );
	void SetDepthTest( eCompareFunc compare, bool writeDepthOnPass );

	int GetTotalTexturePoolCount()															{ return m_totalRenderTargetsMade; }
	int GetTexturePoolFreeCount()															{ return m_totalRenderTargetsMade - m_curActiveRenderTargets; }
	Texture* AcquireRenderTargetMatching( Texture* textureToMatch );
	void ReleaseRenderTarget( Texture* texture );
	void CopyTexture( Texture* destination, Texture* source );
	//void ApplyEffect( Texture* destination, Texture* source, Material* material );
	void StartEffect( Texture* destination, Texture* source, ShaderProgram* shader );
	void StartEffect( Texture* destination, Texture* source, Material* material );
	void EndEffect();

	void BeginCamera( Camera& camera );
	void EndCamera	( const Camera& camera );

	void UpdateFrameData();

	void Draw( int numVertices, int vertexOffset = 0 );
	void DrawIndexed( int indexCount, int indexOffset = 0, int vertexOffset = 0 );
	void DrawMesh( GPUMesh* mesh );

	// Binding State
	void BindMaterial( Material* material );
	void BindShader( Shader* shader );
	void BindShaderByName( std::string shaderName );
	void BindShaderByPath( const char* filePath );
	void BindShaderProgram( ShaderProgram* shader );
	void BindShaderProgram( const char* fileName );
	void BindDiffuseTexture( const Texture* constTexture );
	void BindNormalTexture( const Texture* constTexture );
	void BindSpecGlossEmissiveTexture( const Texture* constTexture );
	void BindTexture( uint slot, const Texture* constTexture );
	void BindSampler( uint slot, Sampler* sampler );

	// Resource Creation
	Shader* GetOrCreateShader( const char* filename );
	ShaderProgram* GetOrCreateShaderProgram( const char* filename );
	ShaderProgram* GetOrCreateShaderProgramFromSourceString( const char* shaderName, const char* source );
	Texture* CreateOrGetTextureFromFile( const char* filePath );
	Texture* CreateTextureFromColor( const Rgba8& color );
	Texture* GetOrCreateDepthStencil( const IntVec2& outputDimensions );
	Texture* CreateRenderTarget( const IntVec2& outputDimensions );
	BitmapFont* CreateOrGetBitmapFontFromFile( const char* filePath, bool useMetadata = false );
	Sampler* GetOrCreateSampler( eSamplerType filter, eSamplerUVMode mode );

	void ReloadShaders();
	//Texture* CreateTextureFromImage( ... ); for cleaning up D3D calls

	void SetModelMatrix( const Mat44& modelMatrix );
	void SetModelData( const Mat44& modelMatrix, const Rgba8& tint = Rgba8::WHITE, float specularFactor = 0.f, float specularPower = 32.f );
	void SetMaterialData( void* materialData, int dataSize );
	void SetLightData();
	void SetDebugLightData( float diffuseEffect = 1.f, float specularEffect = 1.f, float emissiveEffect = 1.f );

	// Raster state setters
	void SetCullMode( eCullMode cullMode );
	void SetFillMode( eFillMode fillMode );
	void SetFrontFaceWindOrder( bool windCCW );

	// Light setters
	void SetAmbientColor( const Rgba8& color );
	void SetAmbientColor( const Vec3& color );
	void SetAmbientIntensity( EventArgs* args );
	void SetAmbientIntensity( float intensity );
	void SetAmbientLight( EventArgs* args );
	void SetAmbientLight( const Rgba8& color, float intensity );
	void SetAmbientLight( const Vec3& color, float intensity );

	void EnableLight( uint idx, const Light& lightInfo );
	void DisableLight( uint idx );
	void DisableAllLights();

	void SetGamma( float gamma );

	void EnableFog( float nearFogDist, float farFogDist, const Rgba8& fogColor );
	void DisableFog();

	// Accessors
	Texture*	GetBackBuffer();
	IntVec2		GetDefaultBackBufferSize();
	BitmapFont* GetSystemFont() const							{ return m_systemFont; }
	Clock*		GetClock() const								{ return m_gameClock; }
	Shader*		GetShaderByName( std::string shaderName );
	Texture*	GetDefaultWhiteTexture()						{ return m_defaultWhiteTexture; }
	Texture*	GetDefaultFlatTexture()							{ return m_flatNormalMap; }
	Texture*	GetDefaultSpecGlossEmissiveTexture()			{ return m_defaultSpecGlossEmissiveTexture; }

	// Debug methods
	void CycleBlendMode();

	// Template Draw
	template<typename VERTEX_TYPE>
	void DrawVertexArray( int numVertices, const VERTEX_TYPE* vertices )
	{
		// Update a vertex buffer
		size_t dataByteSize = numVertices * sizeof( VERTEX_TYPE );
		size_t elementSize = sizeof( VERTEX_TYPE );
		VertexBuffer* immediateVBO = GetImmediateVBO<VERTEX_TYPE>();
		immediateVBO->Update( vertices, dataByteSize, elementSize );

		// Bind
		BindVertexBuffer( immediateVBO );

		// Draw
		Draw( numVertices, 0 );
	}

	template<typename VERTEX_TYPE>
	void DrawVertexArray( const std::vector<VERTEX_TYPE>& vertices )
	{
		GUARANTEE_OR_DIE( vertices.size() > 0, "Empty vertex array cannot be drawn" );
		DrawVertexArray( (int)vertices.size(), &vertices[0] );
	}

	template<typename VERTEX_TYPE>
	VertexBuffer* GetImmediateVBO();

protected:
	// Binding Inputs
	void BindVertexBuffer( VertexBuffer* vbo );
	void BindIndexBuffer( IndexBuffer* ibo );
	void BindUniformBuffer( uint slot, RenderBuffer* ubo );

	void InitializeDefaultRenderObjects();
	void InitializeViewport( const IntVec2& outputSize );
	void UpdateAndBindBuffers( Camera& camera );
	void SetupRenderTargetViewWithDepth( ID3D11RenderTargetView* renderTargetView, const Camera& camera );
	ID3D11DepthStencilView* GetDepthStencilViewFromCamera( const Camera& camera );
	std::vector<ID3D11RenderTargetView*> GetRTVsFromCamera( const Camera& camera );
	void ClearCamera( std::vector<ID3D11RenderTargetView*> renderTargetViews, const Camera& camera );
	void ClearScreen( ID3D11RenderTargetView* renderTargetView, const Rgba8& clearColor );
	void ClearDepth( Texture* depthStencilTarget, float depth );
	void ResetRenderObjects();

	Texture* CreateTextureFromFile( const char* filePath );
	Texture* RetrieveTextureFromCache( const char* filePath );
	
	void CreateBlendStates();

	void SetRasterState( eFillMode fillMode, eCullMode cullMode, bool windCCW );

	BitmapFont* RetrieveBitmapFontFromCache( const std::string& filePath );

	void CreateDebugModule();
	void CreateDefaultRasterState();
	void DestroyDebugModule();
	void ReportLiveObjects();

	void FinalizeContext();

public:
	// SD2 TODO: Move to D3D11Common.hpp
	ID3D11Device* m_device				= nullptr;
	ID3D11DeviceContext* m_context		= nullptr;		// immediate context
	SwapChain* m_swapchain				= nullptr;		// gives us textures that we can draw that the user can see

	void* m_debugModule					= nullptr;
	IDXGIDebug* m_debug					= nullptr;

	VertexBuffer* m_immediateVBOPCU		= nullptr;
	VertexBuffer* m_immediateVBOPCUTBN	= nullptr;
	VertexBuffer* m_immediateVBOFont	= nullptr;
	IndexBuffer* m_immediateIBO			= nullptr;
	RenderBuffer* m_frameUBO			= nullptr;
	RenderBuffer* m_modelMatrixUBO		= nullptr;
	RenderBuffer* m_materialUBO			= nullptr;
	RenderBuffer* m_lightUBO			= nullptr;
	RenderBuffer* m_debugLightUBO		= nullptr;

protected:
	Clock* m_gameClock								= nullptr;

	// Textures
	std::vector<Texture*> m_loadedTextures;
	std::map<std::string, BitmapFont*> m_loadedBitmapFonts;
	BitmapFont* m_systemFont						= nullptr;
	
	int m_totalRenderTargetsMade = 0;
	int m_curActiveRenderTargets = 0;
	std::vector<Texture*> m_renderTargetPool;
	
	ID3D11Buffer* m_lastVBOHandle					= nullptr;
	VertexBuffer* m_lastBoundVBO					= nullptr;
	ID3D11Buffer* m_lastIBOHandle					= nullptr;

	Mat44 m_modelMatrix								= Mat44::IDENTITY;

	// Shaders
	ShaderProgram* m_defaultShaderProgram			= nullptr;
	ShaderProgram* m_currentShaderProgram			= nullptr;
	std::vector<ShaderProgram*> m_loadedShaderPrograms;
	std::vector<Shader*> m_loadedShaders;

	std::vector<Sampler*> m_loadedSamplers;

	Sampler* m_defaultSampler						= nullptr;
	
	// Default textures
	Texture* m_defaultWhiteTexture					= nullptr;
	Texture* m_flatNormalMap						= nullptr;
	Texture* m_defaultSpecGlossEmissiveTexture		= nullptr;
	Texture* m_defaultDepthBuffer					= nullptr;

	// Lighting
	Vec3 m_ambientLightColor						= Vec3::ONE;
	float m_ambientLightIntensity					= 1.f;
	Light m_lights[MAX_LIGHTS];
	float m_gamma									= 2.2f;
	Fog m_linearFog;

	// ID3D states
	ID3D11BlendState* m_alphaBlendState				= nullptr;
	ID3D11BlendState* m_additiveBlendState			= nullptr;
	ID3D11BlendState* m_disabledBlendState			= nullptr;
	eBlendMode m_currentBlendMode					= eBlendMode::ALPHA;

	ID3D11RasterizerState* m_currentRasterState		= nullptr;
	ID3D11RasterizerState* m_defaultRasterState		= nullptr;
	
	ID3D11DepthStencilState* m_currentDepthStencilState = nullptr;

	bool m_isDrawing								= false;

	Camera* m_effectCamera = nullptr;
};

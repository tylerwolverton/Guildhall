#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
constexpr int USER_TEXTURE_SLOT_START = 8;


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
struct Vertex_PCU;
struct Vertex_PCUTBN;
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
enum class eFillMode : uint;
enum class eCullMode : uint;


//-----------------------------------------------------------------------------------------------
enum class eBlendMode
{
	DISABLED,
	ALPHA,
	ADDITIVE,
};


//-----------------------------------------------------------------------------------------------
// Should this be controlled by 2 separate enums?
enum class eSampler
{
	POINT_CLAMP,
	LINEAR_CLAMP,
	POINT_WRAP,
};


//-----------------------------------------------------------------------------------------------
enum eBufferSlot
{
	UBO_FRAME_SLOT = 0,
	UBO_CAMERA_SLOT = 1,
	UBO_MODEL_MATRIX_SLOT = 2,
	UBO_LIGHT_SLOT = 3,
	UBO_MATERIAL_SLOT = 5,
};


//-----------------------------------------------------------------------------------------------
struct FrameData
{
	float systemTimeSeconds;
	float systemDeltaTimeSeconds;
	
	float nearFogDistance = 50.f;
	float farFogDistance = 100.f;

	Vec4 fogColor;
	
	float gamma = 2.2f;
	float padding0;
	float padding1;
	float padding2;
};


//-----------------------------------------------------------------------------------------------
struct ModelData
{
	Mat44 modelMatrix;
	float tint[4];

	float specularFactor;
	float specularPower;

	float padding0;
	float padding1;
};


//-----------------------------------------------------------------------------------------------
struct Light
{
	Vec3 position = Vec3::ONE;
	float pad00;
	
	Vec3 direction = Vec3( 0.f, 0.f, -1.f );
	float isDirectional = 0.f;

	Vec3 color = Vec3::ONE;
	float intensity = 0.f;

	Vec3 attenuation = Vec3( 0.f, 1.f, 0.f );
	float halfCosOfInnerAngle = -1.f;

	Vec3 specularAttenuation = Vec3( 0.f, 1.f, 0.f );
	float halfCosOfOuterAngle = -1.f;
};

struct LightData
{
	Vec4 ambientLight;
	Light lights[MAX_LIGHTS];
};


//-----------------------------------------------------------------------------------------------
struct Fog
{
	float nearFogDistance = 999999.f;
	float farFogDistance = 999999.f;

	Vec4 fogColor = Rgba8::BLACK.GetAsRGBAVector();
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
	void SetSampler( eSampler sampler );
	void SetDepthTest( eCompareFunc compare, bool writeDepthOnPass );

	void ClearScreen( ID3D11RenderTargetView* renderTargetView, const Rgba8& clearColor );
	void ClearDepth( Texture* depthStencilTarget, float depth );

	Texture* AcquireRenderTargetMatching( Texture* textureToMatch );
	void ReleaseRenderTarget( Texture* texture );
	void CopyTexture( Texture* destination, Texture* source );

	void BeginCamera( Camera& camera );
	void EndCamera	( const Camera& camera );

	void UpdateFrameData();

	void Draw( int numVertices, int vertexOffset = 0 );
	void DrawIndexed( int indexCount, int indexOffset = 0, int vertexOffset = 0 );

	void DrawVertexArray( int numVertices, const Vertex_PCU* vertices );
	void DrawVertexArray( const std::vector<Vertex_PCU>& vertices );
	void DrawMesh( GPUMesh* mesh );
	
	// Binding Inputs
	void BindVertexBuffer( VertexBuffer* vbo );
	void BindIndexBuffer( IndexBuffer* ibo );
	void BindUniformBuffer( uint slot, RenderBuffer* ubo );

	// Binding State
	void BindShader( Shader* shader );
	void BindShader( const char* fileName );
	void BindDiffuseTexture( const Texture* constTexture );
	void BindNormalTexture( const Texture* constTexture );
	void BindTexture( uint slot, const Texture* constTexture );
	void BindSampler( Sampler* sampler );

	// Resource Creation
	Shader* GetOrCreateShader( const char* filename );
	Shader* GetOrCreateShaderFromSourceString( const char* shaderName, const char* source );
	Texture* CreateOrGetTextureFromFile( const char* filePath );
	Texture* CreateTextureFromColor( const Rgba8& color );
	Texture* GetOrCreateDepthStencil( const IntVec2& outputDimensions );
	Texture* CreateRenderTarget( const IntVec2& outputDimensions );
	BitmapFont* CreateOrGetBitmapFontFromFile( const char* filePath );

	void ReloadShaders();
	//Texture* CreateTextureFromImage( ... ); for cleaning up D3D calls

	void SetModelData( const Mat44& modelMatrix, const Rgba8& tint = Rgba8::WHITE, float specularFactor = 0.f, float specularPower = 32.f );
	void SetMaterialData( void* materialData, int dataSize );
	//void SetDebugRenderMaterialData( const Rgba8& startTint = Rgba8::WHITE, const Rgba8& endTint = Rgba8::WHITE, float tintRatio = 0.f );
	void SetLightData();

	// Raster state setters
	void SetCullMode( eCullMode cullMode );
	void SetFillMode( eFillMode fillMode );
	void SetFrontFaceWindOrder( bool windCCW );

	// Light setters
	void SetAmbientColor( const Rgba8& color );
	void SetAmbientColor( const Vec3& color );
	void SetAmbientIntensity( float intensity );
	void SetAmbientLight( const Rgba8& color, float intensity );
	void SetAmbientLight( const Vec3& color, float intensity );

	void EnableLight( uint idx, const Light& lightInfo );
	void DisableLight( uint idx );
	void DisableAllLights();

	void SetGamma( float gamma );

	void EnableFog( float nearFogDist, float farFogDist, const Rgba8& fogColor );
	void DisableFog();

	// Accessors
	Texture* GetBackBuffer();
	IntVec2 GetDefaultBackBufferSize();
	Shader* GetCurrentShader() const					{ return m_currentShader; }
	BitmapFont* GetSystemFont() const					{ return m_systemFont; }
	Clock* GetClock() const								{ return m_gameClock; }
	eBlendMode GetBlendMode() const						{ return m_currentBlendMode; }
	eCullMode GetCullMode() const;
	eFillMode GetFillMode() const;
	bool GetFrontFaceWindOrderCCW() const;

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

	void SetRasterState( eFillMode fillMode, eCullMode cullMode, bool windCCW );

	BitmapFont* RetrieveBitmapFontFromCache( const char* filePath );

	void CreateDebugModule();
	void CreateDefaultRasterState();
	void DestroyDebugModule();
	void ReportLiveObjects();

	void FinalizeContext();

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
	RenderBuffer* m_materialUBO		= nullptr;
	RenderBuffer* m_lightUBO		= nullptr;

private:
	Clock* m_gameClock								= nullptr;

	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont*> m_loadedBitmapFonts;
	BitmapFont* m_systemFont						= nullptr;
	
	int m_totalRenderTargetsMade = 0;
	std::vector<Texture*> m_renderTargetPool;


	ID3D11Buffer* m_lastVBOHandle					= nullptr;
	VertexBuffer* m_lastBoundVBO					= nullptr;
	ID3D11Buffer* m_lastIBOHandle					= nullptr;

	Shader* m_defaultShader							= nullptr;
	Shader* m_currentShader							= nullptr;
	std::vector<Shader*> m_loadedShaders;

	Sampler* m_pointClampSampler					= nullptr;
	Sampler* m_linearClampSampler					= nullptr;
	Sampler* m_pointWrapSampler						= nullptr;
	Sampler* m_currentSampler						= nullptr;
	
	Texture* m_defaultWhiteTexture					= nullptr;
	Texture* m_flatNormalMap						= nullptr;
	Texture* m_defaultDepthBuffer					= nullptr;

	Vec3 m_ambientLightColor						= Vec3::ONE;
	float m_ambientLightIntensity					= 1.f;
	Light m_lights[MAX_LIGHTS];
	float m_gamma									= 2.2f;
	Fog m_linearFog;

	ID3D11DepthStencilState* m_currentDepthStencilState = nullptr;

	ID3D11BlendState* m_alphaBlendState				= nullptr;
	ID3D11BlendState* m_additiveBlendState			= nullptr;
	ID3D11BlendState* m_disabledBlendState			= nullptr;
	eBlendMode m_currentBlendMode					= eBlendMode::ALPHA;

	ID3D11RasterizerState* m_currentRasterState		= nullptr;
	ID3D11RasterizerState* m_defaultRasterState		= nullptr;

	bool m_isDrawing								= false;
};

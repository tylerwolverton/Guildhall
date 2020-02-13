#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
class Window;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct IDXGIDebug;
struct ID3D11RenderTargetView;
struct ID3D11Buffer;
class SwapChain;
struct AABB2;
struct OBB2;
struct Capsule2;
class Polygon2;
struct Rgba8;
struct Vertex_PCU;
class Camera;
class Texture;
class Sampler;
class BitmapFont;
class Shader;
class RenderBuffer;
class VertexBuffer;


//-----------------------------------------------------------------------------------------------
enum class eBlendMode
{
	ALPHA,
	ADDITIVE,
};


enum eBufferSlot
{
	UBO_FRAME_SLOT = 0,
	UBO_CAMERA_SLOT = 1,
};


struct FrameData
{
	float SystemTimeSeconds;
	float SystemDeltaTimeSeconds;
	
	float padding[2];
};


//-----------------------------------------------------------------------------------------------
class RenderContext
{
public:
	void Startup( Window* window );
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void SetBlendMode( eBlendMode blendMode );
	void ClearScreen( ID3D11RenderTargetView* renderTargetView, const Rgba8& clearColor );
	void BeginCamera( Camera& camera );
	void EndCamera	( const Camera& camera );

	void UpdateFrameTime( float deltaSeconds );

	void Draw( int numVertices, int vertexOffset = 0 );
	void DrawVertexArray( int numVertices, const Vertex_PCU* vertices );
	void DrawVertexArray( const std::vector<Vertex_PCU>& vertices );
	
	void DrawLine2D( const Vec2& start, const Vec2& end, const Rgba8& color, float thickness );
	void DrawRing2D( const Vec2& center, float radius, const Rgba8& color, float thickness );
	void DrawDisc2D( const Vec2& center, float radius, const Rgba8& color );
	void DrawCapsule2D( const Capsule2& capsule, const Rgba8& color );
	void DrawAABB2( const AABB2& box, const Rgba8& tint );
	void DrawAABB2Outline( const Vec2& center, const AABB2& box, const Rgba8& tint, float thickness );
	void DrawOBB2( const OBB2& box, const Rgba8& tint );
	void DrawOBB2Outline( const Vec2& center, const OBB2& box, const Rgba8& tint, float thickness );
	void DrawPolygon2( const Polygon2& polygon2, const Rgba8& tint );
	void DrawPolygon2( const std::vector<Vec2>& vertexPositions, const Rgba8& tint );
	void DrawPolygon2Outline( const Polygon2& polygon2, const Rgba8& tint, float thickness );
	void DrawPolygon2Outline( const std::vector<Vec2>& vertexPositions, const Rgba8& tint, float thickness );

	static void AppendVertsForArc		( std::vector<Vertex_PCU>& vertexArray, const Vec2& center, float radius, float arcAngleDegrees, float startOrientationDegrees, const Rgba8& tint );
	static void AppendVertsForAABB2D	( std::vector<Vertex_PCU>& vertexArray, const AABB2& spriteBounds,	const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	static void AppendVertsForOBB2D		( std::vector<Vertex_PCU>& vertexArray, const OBB2& spriteBounds,	const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	static void AppendVertsForOBB2D		( std::vector<Vertex_PCU>& vertexArray, const Vec2& bottomLeft, const Vec2& bottomRight, const Vec2& topLeft, const Vec2& topRight, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	static void AppendVertsForCapsule2D	( std::vector<Vertex_PCU>& vertexArray, const Capsule2& capsule,	const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	static void AppendVertsForPolygon2	( std::vector<Vertex_PCU>& vertexArray, const std::vector<Vec2>& vertexPositions, const Rgba8& tint, const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );

	Texture* GetFrameColorTarget();

	// Binding Inputs
	void BindVertexBuffer( VertexBuffer* vbo );
	
	void BindUniformBuffer( uint slot, RenderBuffer* ubo );

	// Binding State
	void BindShader( Shader* shader );
	void BindShader( const char* fileName );

	// Resource Creation
	Shader* GetOrCreateShader( char const* filename );

	Texture* CreateOrGetTextureFromFile( const char* filePath );
	void BindTexture( const Texture* constTexture );
	void BindSampler( Sampler* sampler );

	BitmapFont* CreateOrGetBitmapFontFromFile( const char* filePath );

	Texture* CreateTextureFromColor( const Rgba8& color );
	//Texture* CreateTextureFromImage( ... ); for cleaning up D3D calls

private:
	Texture* CreateTextureFromFile( const char* filePath );
	Texture* RetrieveTextureFromCache( const char* filePath );

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

	RenderBuffer* m_frameUBO		= nullptr;

private:
	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont*> m_loadedBitmapFonts;

	ID3D11Buffer* m_lastVBOHandle = nullptr;

	Shader* m_defaultShader = nullptr;
	Shader* m_currentShader = nullptr;
	std::vector<Shader*> m_loadedShaders;

	Sampler* m_defaultSampler = nullptr;
	Texture* m_defaultWhiteTexture = nullptr;

	bool m_isDrawing = false;
};

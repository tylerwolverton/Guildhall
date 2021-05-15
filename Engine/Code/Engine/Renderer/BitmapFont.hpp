#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"

#include <string>


//------------------------------------------------------------------------------------------------
struct Vertex_PCU;
struct VertexFont;
struct AABB2;
class Texture;


//------------------------------------------------------------------------------------------------
struct GlyphData
{
	int spriteIdx = 0;
	float preAdvance = 0.f;
	float glyphAspect = 0.f;
	float postAdvance = 0.f;
	float yOffsetNormalized = 0.f;
	float heightNormalized = 1.f;
};


//------------------------------------------------------------------------------------------------
class BitmapFont
{
	friend class RenderContext; // Only the RenderContext can create new BitmapFont objects!

private:
	explicit BitmapFont( const std::string& fontName, const Texture* fontTexture );
	explicit BitmapFont( const std::string& fontName, const Texture* fontTexture, const std::string& metaDataFilePath );

public:
	~BitmapFont();

	const Texture* GetTexture() const;
	const std::string GetFontName() const												{ return m_fontName; };

	void AppendVertsForText2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight, 
							   const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f );

	void AppendVertsForText2D( std::vector<VertexFont>& vertexArray, const Vec2& textMins, float cellHeight,
							   const std::string& text, const Rgba8& tint = Rgba8::WHITE, const Vec4& specialEffects = Vec4::ZERO );

	void AppendVertsAndIndicesForText2D( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indexArray, const Vec2& textMins, float cellHeight,
										 const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f );

	void AppendVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, float cellHeight,
								 const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f,
								 const Vec2& alignment = ALIGN_CENTERED );

	Vec2 GetDimensionsForText2D( float cellHeight, const std::string& text, float cellAspect = 1.f );
	Vec2 GetDimensionsForProportionalText2D( float cellHeight, const std::string& text );

protected:
	float GetGlyphAspect( int glyphUnicode ) const; 

	void AppendVertsForText2DWithMetadata( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight,
										   const std::string& text, const Rgba8& tint = Rgba8::WHITE );

protected:
	std::string	m_fontName;
	SpriteSheet* m_glyphSpriteSheet = nullptr;
	std::map<int, GlyphData> m_glyphMetaData;
};

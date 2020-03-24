#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <string>


//------------------------------------------------------------------------------------------------
struct Vertex_PCU;
struct AABB2;
class Texture;


//------------------------------------------------------------------------------------------------
class BitmapFont
{
	friend class RenderContext; // Only the RenderContext can create new BitmapFont objects!

private:
	explicit BitmapFont( const char* fontName, const Texture* fontTexture );

public:
	const Texture* GetTexture() const;
	const std::string GetFontName() const												{ return m_fontName; };

	void AppendVertsForText2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight, 
							   const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f );
	void AppendVertsAndIndicesForText2D( std::vector<Vertex_PCU>& vertexArray, std::vector<uint>& indexArray, const Vec2& textMins, float cellHeight,
										 const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f );

	void AppendVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, float cellHeight,
								 const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f,
								 const Vec2& alignment = ALIGN_CENTERED );

	Vec2 GetDimensionsForText2D( float cellHeight, const std::string& text, float cellAspect = 1.f );

protected:
	float GetGlyphAspect( int glyphUnicode ) const; // For now this will always return 1.0f!!!

protected:
	std::string	m_fontName;
	SpriteSheet	m_glyphSpriteSheet;
};

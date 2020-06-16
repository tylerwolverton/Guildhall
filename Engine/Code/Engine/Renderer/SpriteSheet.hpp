#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

#include <string>
#include <vector>
#include <map>

class SpriteSheet
{
public:
	explicit SpriteSheet( const Texture& texture, const IntVec2& simpleGridLayout );
	//explicit SpriteSheet( const std::string& name, const Texture& texture, const IntVec2& simpleGridLayout );
	~SpriteSheet();

	const Texture&			GetTexture() const																	{ return m_texture; }
	IntVec2					GetDimensions() const																{ return m_dimensions; }
	int						GetNumSprites() const																{ return (int)m_spriteDefs.size(); }
	const SpriteDefinition& GetSpriteDefinition( int spriteIndex ) const;
	void					GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const;
	void					GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, const IntVec2& spriteCoords ) const;

	static void CreateAndAddToMap( const std::string& name, const Texture& texture, const IntVec2& simpleGridLayout );
	static SpriteSheet* GetSpriteSheet( std::string spriteSheetName );
	static void DeleteSpriteSheets();

public:
	static std::map< std::string, SpriteSheet* > s_definitions;

private:
	int GetSpriteIndexFromSpriteCoords( const IntVec2& spriteCoords ) const;

private:
	const Texture&					m_texture;
	std::vector<SpriteDefinition>	m_spriteDefs;
	IntVec2							m_dimensions;
};
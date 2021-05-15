#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

#include <string>
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
struct AABB2;


//-----------------------------------------------------------------------------------------------
class SpriteSheet
{
public:
	explicit SpriteSheet( const Texture& texture, const IntVec2& simpleGridLayout );
	explicit SpriteSheet( const std::string& name, const Texture& texture, const IntVec2& simpleGridLayout );
	explicit SpriteSheet( const Texture& texture, const std::vector<AABB2>& uvBoundsPerSprite );
	explicit SpriteSheet( const std::string& name, const Texture& texture, const std::vector<AABB2>& uvBoundsPerSprite );
	~SpriteSheet();

	const Texture&			GetTexture() const																	{ return m_texture; }
	IntVec2					GetDimensions() const																{ return m_dimensions; }
	int						GetNumSprites() const																{ return (int)m_spriteDefs.size(); }
	const SpriteDefinition& GetSpriteDefinition( int spriteIndex ) const;
	void					GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const;
	void					GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, const IntVec2& spriteCoords ) const;

	// Static methods
	static SpriteSheet* CreateAndRegister( const std::string& name, const Texture& texture, const IntVec2& simpleGridLayout );
	static SpriteSheet* CreateAndRegister( const Texture& texture, const IntVec2& simpleGridLayout );
	static SpriteSheet* GetSpriteSheetByName( std::string spriteSheetName );
	static SpriteSheet* GetSpriteSheetByPath( std::string spriteSheetPath );

public:
	static std::vector< SpriteSheet* > s_definitions;

private:
	int GetSpriteIndexFromSpriteCoords( const IntVec2& spriteCoords ) const;

private:
	std::string						m_name;

	const Texture&					m_texture;
	std::vector<SpriteDefinition>	m_spriteDefs;
	IntVec2							m_dimensions;
};

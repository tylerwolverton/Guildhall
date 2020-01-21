#include "Game/TileDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::vector<TileDefinition> TileDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
TileDefinition::TileDefinition( const AABB2& uvCoords, const Rgba8& tintColor, bool isSolid, bool stopsBullets )
	: m_uvCoords( uvCoords )
	, m_tintColor( tintColor )
	, m_isSolid( isSolid )
	, m_stopsBullets( stopsBullets )
{
}


//-----------------------------------------------------------------------------------------------
TileDefinition::~TileDefinition()
{
}


//-----------------------------------------------------------------------------------------------
void TileDefinition::PopulateDefinitions()
{
	s_definitions.reserve( NUM_TILE_TYPES );

	Texture* tileTexture = g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" );
	SpriteSheet* tileSpriteSheet = new SpriteSheet( *tileTexture, IntVec2( 8, 8 ) );
	Vec2 uvAtMins, uvAtMaxs;

	// TILE_TYPE_GRASS
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 9 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, false ) ); 
	// TILE_TYPE_STONE
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 34 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, true, true ) );
	// TILE_TYPE_MUD
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 21 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, false ) );
	// TILE_TYPE_SAND
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 14 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, false ) );
	// TILE_TYPE_WET_SAND
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 23 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, false ) );
	// TILE_TYPE_CONCRETE
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 51 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, true, true ) );
	// TILE_TYPE_WATER
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 61 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, true ) );
	// TILE_TYPE_LAVA
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 62 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, false ) );
	// TILE_TYPE_PURPLE_BRICK
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 42 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, false ) );
	// TILE_TYPE_WOOD
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 49 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8( 255, 255, 255, 100 ), true, true ) );
	// TILE_TYPE_MARBLE
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 22 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, false ) );
	// TILE_TYPE_EXIT
	tileSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxs, 57 );
	s_definitions.push_back( TileDefinition( AABB2( uvAtMins, uvAtMaxs ), Rgba8::WHITE, false ) );

	delete tileSpriteSheet;
	tileSpriteSheet = nullptr;
}

#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
SpriteSheet::SpriteSheet( const Texture& texture, const IntVec2& simpleGridLayout )
	: m_texture( texture )
	, m_dimensions( simpleGridLayout )
{
	float uStep = 1.f / (float)simpleGridLayout.x;
	float vStep = 1.f / (float)simpleGridLayout.y;

	int currentXSpritePos = 0;
	int currentYSpritePos = simpleGridLayout.y - 1;

	for ( int spriteIndex = 0; spriteIndex < simpleGridLayout.x * simpleGridLayout.y; ++spriteIndex )
	{
		Vec2 uvAtMins = Vec2( (float)currentXSpritePos * uStep, (float)currentYSpritePos * vStep );
		Vec2 uvAtMaxs = Vec2( (float)( currentXSpritePos + 1 ) * uStep, (float)( currentYSpritePos + 1) * vStep );

		m_spriteDefs.push_back( SpriteDefinition( *this, spriteIndex, uvAtMins, uvAtMaxs ) );

		++currentXSpritePos;
		if ( currentXSpritePos >= simpleGridLayout.x )
		{
			currentXSpritePos = 0;
			--currentYSpritePos;
		}
	}
}


//-----------------------------------------------------------------------------------------------
const SpriteDefinition& SpriteSheet::GetSpriteDefinition( int spriteIndex ) const
{
	return m_spriteDefs[spriteIndex];
}


//-----------------------------------------------------------------------------------------------
void SpriteSheet::GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, int spriteIndex ) const
{
	if ( spriteIndex < 0 )
	{
		return;
	}

	SpriteDefinition spriteDef = m_spriteDefs[spriteIndex];

	spriteDef.GetUVs( out_uvAtMins, out_uvAtMaxs );
}


//-----------------------------------------------------------------------------------------------
void SpriteSheet::GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxs, const IntVec2& spriteCoords ) const
{
	GetSpriteUVs( out_uvAtMins, out_uvAtMaxs, GetSpriteIndexFromSpriteCoords( spriteCoords ) );
}


//-----------------------------------------------------------------------------------------------
int SpriteSheet::GetSpriteIndexFromSpriteCoords( const IntVec2& spriteCoords ) const
{
	GUARANTEE_OR_DIE( spriteCoords.x >= 0 && spriteCoords.y >= 0, "Negative index for sprite requested" );

	int spriteIndex = spriteCoords.x + ( spriteCoords.y * m_dimensions.x );

	GUARANTEE_OR_DIE( spriteIndex < GetNumSprites(), "Sprite index out of bounds for image" );

	return spriteIndex;
}

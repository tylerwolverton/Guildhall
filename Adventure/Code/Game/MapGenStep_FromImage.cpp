#include "Game/MapGenStep_FromImage.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Tile.hpp"
#include "Game/Map.hpp"

#include <cmath>


//-----------------------------------------------------------------------------------------------
MapGenStep_FromImage::MapGenStep_FromImage( const XmlElement& mapGenStepXmlElem )
	: MapGenStep( mapGenStepXmlElem )
	, m_numRotations( 0 )
	, m_alignmentX( 0.f, 1.f )
	, m_alignmentY( 0.f, 1.f )
{
	m_imageFilePath = ParseXmlAttribute( mapGenStepXmlElem, "imageFilePath", m_imageFilePath );
	m_numRotations = ParseXmlAttribute( mapGenStepXmlElem, "numRotations", m_numRotations );
	m_alignmentX = ParseXmlAttribute( mapGenStepXmlElem, "alignmentX", m_alignmentX );
	m_alignmentY = ParseXmlAttribute( mapGenStepXmlElem, "alignmentY", m_alignmentY );
	m_chanceToMirror = ParseXmlAttribute( mapGenStepXmlElem, "chanceToMirror", m_chanceToMirror );
}


//-----------------------------------------------------------------------------------------------
void MapGenStep_FromImage::RunStepOnce( Map& map )
{
	float chancePerTile = m_chancePerTile.GetRandomInRange( g_game->m_rng );
	Vec2 uvAlignment( m_alignmentX.GetRandomInRange( g_game->m_rng ), m_alignmentY.GetRandomInRange( g_game->m_rng ) );

	Image image( m_imageFilePath.c_str() );
	int numRotations = m_numRotations.GetRandomInRange( g_game->m_rng );
	bool isMirrored = g_game->m_rng->RollPercentChance( m_chanceToMirror );

	Vec2 mapDimensions( (float)map.m_width, (float)map.m_height );
	Vec2 differenceInSize( mapDimensions - Vec2( (float)image.GetDimensions( numRotations ).x, (float)image.GetDimensions( numRotations ).y ) );
	
	IntVec2 bottomLeftWorldTileCoords( (int)std::round( differenceInSize.x * uvAlignment.x ),
									   (int)std::round( differenceInSize.y * uvAlignment.y ) );

	for ( int imageYPos = 0; imageYPos < image.GetDimensions( numRotations ).y; ++imageYPos )
	{
		for (int imageXPos = 0; imageXPos < image.GetDimensions( numRotations ).x; ++imageXPos)
		{
			Tile* tile = map.GetTileFromTileCoords( bottomLeftWorldTileCoords + IntVec2( imageXPos, imageYPos ) );
			if ( tile != nullptr
				 && ( m_ifTileType == nullptr || m_ifTileType == tile->m_tileDef ) )
			{
				Rgba8 imageTexelColor = image.GetTexelColor( imageXPos, imageYPos, numRotations, isMirrored );

				if ( imageTexelColor.a == 0 )
				{
					continue;
				}

				TileDefinition* tileDef = TileDefinition::GetTileDefinitionFromImageTexelColor( imageTexelColor );
				GUARANTEE_OR_DIE( tileDef != nullptr, Stringf( "Unknown tile image color (%d, %d, %d) found in image '%s'", imageTexelColor.r, imageTexelColor.g, imageTexelColor.b, m_imageFilePath.c_str() ) );

				if ( g_game->m_rng->RollPercentChance( chancePerTile * ( imageTexelColor.a / 255 ) ) )
				{
					tile->SetTileDef( tileDef );
				}	
			}
		}
	}

}

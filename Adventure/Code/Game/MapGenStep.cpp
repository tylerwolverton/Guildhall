#include "Game/MapGenStep.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/MapGenStep_Mutate.hpp"
#include "Game/MapGenStep_Worm.hpp"
#include "Game/MapGenStep_CellularAutomata.hpp"
#include "Game/MapGenStep_FromImage.hpp"
#include "Game/MapGenStep_RoomsAndPaths.hpp"


//-----------------------------------------------------------------------------------------------
MapGenStep::MapGenStep( const XmlElement& mapGenStepXmlElem )
	: m_numIterations( 1 )
	, m_chancePerTile( .1f, .3f )
{
	m_chanceToRun = ParseXmlAttribute( mapGenStepXmlElem, "chanceToRun", m_chanceToRun );
	m_numIterations = ParseXmlAttribute( mapGenStepXmlElem, "iterations", m_numIterations );

	std::string setTileName;
	setTileName = ParseXmlAttribute( mapGenStepXmlElem, "setTile", setTileName );
	m_setTileType = TileDefinition::GetTileDefinition( setTileName );

	std::string ifTileName;
	ifTileName = ParseXmlAttribute( mapGenStepXmlElem, "ifTile", ifTileName );
	m_ifTileType = TileDefinition::GetTileDefinition( ifTileName );

	m_chancePerTile = ParseXmlAttribute( mapGenStepXmlElem, "chancePerTile", m_chancePerTile );
}


//-----------------------------------------------------------------------------------------------
void MapGenStep::RunStep( Map& map )
{
	if( !g_game->m_rng->RollPercentChance( m_chanceToRun ) )
	{
		return;
	}

	int numIterations = m_numIterations.GetRandomInRange( g_game->m_rng );
	for( int iterationNum = 0; iterationNum < numIterations; ++iterationNum )
	{
		RunStepOnce( map );
	}
}


//-----------------------------------------------------------------------------------------------
MapGenStep* MapGenStep::CreateNewMapGenStep( const XmlElement& mapGenStepXmlElem )
{
	// Check each tile 
	std::string mapGenStepName = mapGenStepXmlElem.Name();
	if ( mapGenStepName == "Worm" )  return new MapGenStep_Worm( mapGenStepXmlElem );
	if ( mapGenStepName == "Mutate" )  return new MapGenStep_Mutate( mapGenStepXmlElem );
	if ( mapGenStepName == "CellularAutomata" )  return new MapGenStep_CellularAutomata( mapGenStepXmlElem );
	if ( mapGenStepName == "FromImage" )  return new MapGenStep_FromImage( mapGenStepXmlElem );
	if ( mapGenStepName == "RoomsAndPaths" )  return new MapGenStep_RoomsAndPaths( mapGenStepXmlElem );

	ERROR_AND_DIE( Stringf( "Map step name '%s' doesn't match any generation steps", mapGenStepName.c_str() ) );
	return nullptr;
}

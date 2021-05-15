#include "Game/LevelThresholdDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::vector<LevelData> LevelThresholdDefinition::m_levelDataThresholds;

//-----------------------------------------------------------------------------------------------
void LevelThresholdDefinition::LoadLevelThresholdsFromXML( const XmlElement& levelThresholdsDefElem )
{
	LevelData firstLevel;
	firstLevel.num = 1;
	firstLevel.xpRequired = 0;
	m_levelDataThresholds.push_back( firstLevel );

	const XmlElement* levelElem = levelThresholdsDefElem.FirstChildElement( "Level" );
	while ( levelElem != nullptr )
	{
		LevelData level;
		level.num =					ParseXmlAttribute( *levelElem, "number", level.num );
		level.xpRequired =			ParseXmlAttribute( *levelElem, "xpRequired", level.xpRequired );
		level.dmgIncrease =			ParseXmlAttribute( *levelElem, "damageIncrease", level.dmgIncrease );
		level.defenseIncrease =		ParseXmlAttribute( *levelElem, "defenseIncrease", level.dmgIncrease );
		level.rangeIncrease =		ParseXmlAttribute( *levelElem, "rangeIncrease", level.rangeIncrease );
		level.atkSpeedIncrease =	ParseXmlAttribute( *levelElem, "attackSpeedIncrease", level.atkSpeedIncrease );
		level.critChanceIncrease =	ParseXmlAttribute( *levelElem, "critChanceIncrease", level.critChanceIncrease );
		
		m_levelDataThresholds.back().xpToNextLevel = level.xpRequired;

		m_levelDataThresholds.push_back( level );

		levelElem = levelElem->NextSiblingElement( "Level" );
	}
}


//-----------------------------------------------------------------------------------------------
LevelData LevelThresholdDefinition::GetLevelForXP( int xp )
{
	for ( int levelIdx = (int)m_levelDataThresholds.size() - 1; levelIdx >= 0; --levelIdx )
	{
		if ( xp >= m_levelDataThresholds[levelIdx].xpRequired )
		{
			return m_levelDataThresholds[levelIdx];
		}
	}

	return m_levelDataThresholds[0];
}


//-----------------------------------------------------------------------------------------------
int LevelThresholdDefinition::GetXPToLevel2()
{
	return m_levelDataThresholds[0].xpToNextLevel;
}


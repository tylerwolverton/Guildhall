#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

#include <vector>

//<Level number = "2" xpRequired = "3" damageIncrease = "1,1" defenseIncrease = "1,1" attackSpeedIncrease = ".1,.1" critChanceIncrease = "0"/>
//-----------------------------------------------------------------------------------------------
struct LevelData
{
	int num = 0;
	int xpRequired = 99999;
	int xpToNextLevel = -1;
	IntVec2 dmgIncrease = IntVec2::ZERO;
	IntVec2 defenseIncrease = IntVec2::ZERO;
	float rangeIncrease = 0.f;
	float atkSpeedIncrease = 0.f;
	float critChanceIncrease = 0.f;
};


//-----------------------------------------------------------------------------------------------
class LevelThresholdDefinition
{
public:
	static void LoadLevelThresholdsFromXML( const XmlElement& levelThresholdsDefElem );

	static LevelData GetLevelForXP( int xp );

	static int GetXPToLevel2();

private:
	static std::vector<LevelData> m_levelDataThresholds;
};

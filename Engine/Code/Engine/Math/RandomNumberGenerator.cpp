#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/RawNoise.hpp"


//-----------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntLessThan( int maxNotInclusive )
{
	unsigned int randomVal = Get1dNoiseUint( m_position++, m_seed );

	return randomVal % maxNotInclusive;
}


//-----------------------------------------------------------------------------------------------
int RandomNumberGenerator::RollRandomIntInRange( int minInclusive, int maxInclusive )
{
	int range = maxInclusive - minInclusive + 1;
	return minInclusive + RollRandomIntLessThan( range );
}


//-----------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatLessThan( float maxNotInclusive )
{
	unsigned int randomVal = Get1dNoiseUint( m_position++, m_seed );

	float scaleFactor = 1.f / ( maxNotInclusive - 1.f );
	float zeroToAlmostOne = scaleFactor * (float)randomVal;

	return zeroToAlmostOne * maxNotInclusive;
}


//-----------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatInRange( float minInclusive, float maxInclusive )
{
	float range = maxInclusive - minInclusive;
	return minInclusive + RollRandomFloatZeroToOneInclusive() * range;
}


//-----------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatZeroToOneInclusive()
{
	unsigned int randomVal = Get1dNoiseUint( m_position++, m_seed );

	float scaleFactor = 1.f / (float)0xFFFFFFFF;
	return (float)randomVal * scaleFactor;
}


//-----------------------------------------------------------------------------------------------
float RandomNumberGenerator::RollRandomFloatZeroToAlmostOne()
{
	unsigned int randomVal = Get1dNoiseUint( m_position++, m_seed );

	float scaleFactor = 1.f / ( 1.f + (float)0xFFFFFFFF );
	return (float)randomVal * scaleFactor;
}


//-----------------------------------------------------------------------------------------------
bool RandomNumberGenerator::RollPercentChance( float probabilityOfReturningTrue )
{
	float randomVal = RollRandomFloatZeroToAlmostOne();

	return probabilityOfReturningTrue > randomVal;
}


//-----------------------------------------------------------------------------------------------
Vec2 RandomNumberGenerator::RollRandomDirection2D()
{
	float randomX = RollRandomFloatInRange( -1.f, 1.f );
	float randomY = RollRandomFloatInRange( -1.f, 1.f );
	Vec2 randomDir( randomX, randomY );

	return randomDir.GetNormalized();
}


//-----------------------------------------------------------------------------------------------
void RandomNumberGenerator::Reset( unsigned int seed )
{
	m_seed = seed;
	m_position = 0;
}

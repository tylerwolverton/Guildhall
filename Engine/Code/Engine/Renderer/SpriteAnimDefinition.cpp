#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition::SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex, float durationSeconds, SpriteAnimPlaybackType playbackType )
	: m_spriteSheet( sheet )
	, m_durationSeconds( durationSeconds )
	, m_playbackType( playbackType )
{
	if ( startSpriteIndex < endSpriteIndex )
	{
		for ( int spriteIndex = startSpriteIndex; spriteIndex <= endSpriteIndex; ++spriteIndex )
		{
			m_spriteIndexes.push_back( spriteIndex );
		}
	}
	else
	{
		for ( int spriteIndex = startSpriteIndex; spriteIndex >= endSpriteIndex; --spriteIndex )
		{
			m_spriteIndexes.push_back( spriteIndex );
		}
	}
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition::SpriteAnimDefinition( const SpriteSheet& sheet, std::vector<int> spriteIndexes, float fps, SpriteAnimPlaybackType playbackType )
	: m_spriteSheet( sheet )
	, m_spriteIndexes( spriteIndexes )
	, m_playbackType( playbackType )
{
	// convert fps to duration seconds
	m_durationSeconds = (float)m_spriteIndexes.size() / fps;
}


//-----------------------------------------------------------------------------------------------
const SpriteDefinition& SpriteAnimDefinition::GetSpriteDefAtTime( float seconds ) const
{
	// Only 1 sprite in this animation
	if( (int)m_spriteIndexes.size() == 1 ) 
	{
		return m_spriteSheet.GetSpriteDefinition( m_spriteIndexes[0] );
	}
	
	// Determine number of frames based on the playback type
	int numFrames = (int)m_spriteIndexes.size();
	if ( m_playbackType == SpriteAnimPlaybackType::PINGPONG )
	{
		numFrames = ( 2 * (int)m_spriteIndexes.size() ) - 2;
	}

	float secondsPerFrame = m_durationSeconds / numFrames;
	int frameIndex = (int)( seconds / secondsPerFrame );

	int spriteIndex = m_spriteIndexes[0];
	switch ( m_playbackType )
	{
		case SpriteAnimPlaybackType::ONCE:
		{
			spriteIndex = m_spriteIndexes[ ClampMinMaxInt( frameIndex, 0, numFrames - 1 ) ];
		}
		break;

		case SpriteAnimPlaybackType::LOOP:
		{
			frameIndex = PositiveMod( frameIndex, numFrames );
			spriteIndex = m_spriteIndexes[ frameIndex ];
		}
		break;
		
		case SpriteAnimPlaybackType::PINGPONG:
		{
			frameIndex = PositiveMod( frameIndex, numFrames );

			// Return sprite index if in 'ping' section
			// -> 0 1 2 3 4 <- 3 2 1
			if ( frameIndex <= numFrames / 2 )
			{
				spriteIndex = m_spriteIndexes[frameIndex];
				break;
			}

			// Return where we are in the 'pong' section
			// 0 1 2 3 4 -> 3 2 1 <-
			int pongIndex = frameIndex - ( numFrames / 2 );
			int pingIndex = (numFrames / 2) - pongIndex;
			spriteIndex = m_spriteIndexes[ pingIndex ];
		}
		break;
	}

	return m_spriteSheet.GetSpriteDefinition( spriteIndex );
}

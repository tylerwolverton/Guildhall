#pragma once
#include <vector>

//------------------------------------------------------------------------------------------------
class SpriteSheet;
class SpriteDefinition;


//------------------------------------------------------------------------------------------------
enum class SpriteAnimPlaybackType
{
	ONCE,		// for 5-frame anim, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
	LOOP,		// for 5-frame anim, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
	PINGPONG,	// for 5-frame anim, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};


//------------------------------------------------------------------------------------------------
class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition( const SpriteSheet& sheet, int startSpriteIndex, int endSpriteIndex,
						  float durationSeconds, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP );

	SpriteAnimDefinition( const SpriteSheet& sheet, std::vector<int> spriteIndexes,
						  float fps, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP );

	const SpriteDefinition& GetSpriteDefAtTime( float seconds ) const;
	int GetFrameIndexAtTime( float seconds ) const;
	int GetNumFrames() const;
	int GetLastSpriteIndex() const;
	
	void SetSpeedModifier( float modifier );

private:
	std::vector<int>		m_spriteIndexes;
	const SpriteSheet&		m_spriteSheet;
	float					m_defaultDurationSeconds = 1.f;
	float					m_durationSeconds = 1.f;
	SpriteAnimPlaybackType	m_playbackType = SpriteAnimPlaybackType::LOOP;
};

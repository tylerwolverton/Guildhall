#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition::SpriteAnimationSetDefinition( SpriteSheet* spriteSheet, const XmlElement& spriteAnimSetDefElem, float defaultFPS )
	: m_spriteSheet( spriteSheet )
	, m_defaultFPS( defaultFPS )
{
	m_name = spriteAnimSetDefElem.Name();

	AddDirectionAnimation( "up",			Vec2( 0.f, 1.f ),						spriteAnimSetDefElem );
	AddDirectionAnimation( "upLeft",		Vec2( -SQRT_2_OVER_2, SQRT_2_OVER_2 ),	spriteAnimSetDefElem );
	AddDirectionAnimation( "left",			Vec2( -1.f, 0.f ),						spriteAnimSetDefElem );
	AddDirectionAnimation( "downLeft",		Vec2( -SQRT_2_OVER_2, -SQRT_2_OVER_2 ),	spriteAnimSetDefElem );
	AddDirectionAnimation( "down",			Vec2( 0.f, -1.f ),						spriteAnimSetDefElem );
	AddDirectionAnimation( "downRight",		Vec2( SQRT_2_OVER_2, -SQRT_2_OVER_2 ),	spriteAnimSetDefElem );
	AddDirectionAnimation( "right",			Vec2( 1.f, 0.f ),						spriteAnimSetDefElem );
	AddDirectionAnimation( "upRight",		Vec2( SQRT_2_OVER_2, SQRT_2_OVER_2 ),	spriteAnimSetDefElem );
}


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition::~SpriteAnimationSetDefinition()
{
	PTR_MAP_SAFE_DELETE( m_directionSpriteAnims );
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* SpriteAnimationSetDefinition::GetSpriteAnimationDefForDirection( const Vec2& direction )
{
	Vec2 animDirection = direction;
	if ( IsNearlyEqual( direction, Vec2::ZERO ) )
	{
		animDirection = m_lastDirection;
	}
	else
	{
		m_lastDirection = direction;
	}

	float maxDotProduct = -99999.f;
	std::string closestAnimName = "";
	for ( auto it = m_directionSpriteAnims.begin(); it != m_directionSpriteAnims.end(); ++it )
	{ 
		float dirDotProduct = DotProduct2D( direction, it->second->facingDirection );

		if ( dirDotProduct > maxDotProduct )
		{
			maxDotProduct = dirDotProduct;
			closestAnimName = it->first;
		}
	}

	auto mapIter = m_directionSpriteAnims.find( closestAnimName );
	if ( mapIter == m_directionSpriteAnims.cend() )
	{
		return nullptr;
	}

	return mapIter->second->animDef;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationSetDefinition::AddFrameEvent( int frameNum, const std::string& eventName )
{
	m_frameToEventNames[frameNum] = eventName;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationSetDefinition::FireFrameEvent( int frameNum, Entity* parent )
{
	auto iter = m_frameToEventNames.find( frameNum );
	if ( iter == m_frameToEventNames.end() )
	{
		return;
	}

	if ( parent == nullptr )
	{
		g_eventSystem->FireEvent( iter->second );
	}
	else
	{
		parent->FireScriptEvent( iter->second );
	}
}


//-----------------------------------------------------------------------------------------------
int SpriteAnimationSetDefinition::GetNumFrames() const
{
	if ( m_directionSpriteAnims.empty() )
	{
		return 0;
	}

	return m_directionSpriteAnims.begin()->second->animDef->GetNumFrames();
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationSetDefinition::AdjustAnimationSpeed( float deltaSpeedModifier )
{
	m_curSpeedModifier += deltaSpeedModifier;

	for ( auto& anim : m_directionSpriteAnims )
	{
		anim.second->animDef->SetSpeedModifier( m_curSpeedModifier );
	}
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationSetDefinition::AddDirectionAnimation( const std::string& animName, const Vec2& facingDir, const XmlElement& spriteAnimSetDefElem )
{
	if ( spriteAnimSetDefElem.Attribute( animName.c_str() ) )
	{
		//Ints spriteIndexes;
		IntRange spriteIndexes;
		spriteIndexes = ParseXmlAttribute( spriteAnimSetDefElem, animName.c_str(), spriteIndexes );
		if ( spriteIndexes.min > spriteIndexes.max )
		{
			g_devConsole->PrintError( Stringf( "Animation '%s' %s has no sprite indexes defined.", m_name.c_str(), animName.c_str() ) );
		}
		else
		{
			float fps = ParseXmlAttribute( spriteAnimSetDefElem, "fps", m_defaultFPS );
			std::string typeStr = ParseXmlAttribute( spriteAnimSetDefElem, "playbackType", "loop" );
			SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP;
			if ( IsEqualIgnoreCase( typeStr, "pingpong" ) )
			{
				playbackType = SpriteAnimPlaybackType::PINGPONG;
			}
			if ( IsEqualIgnoreCase( typeStr, "once" ) )
			{
				playbackType = SpriteAnimPlaybackType::ONCE;
			}

			DirectionAnimation* dirAnim = new DirectionAnimation();
			dirAnim->animDef = new SpriteAnimDefinition( *m_spriteSheet, spriteIndexes.GetAsIntVector(), fps, playbackType );
			dirAnim->facingDirection = facingDir;

			m_directionSpriteAnims[animName] = dirAnim;
		}
	}
}
